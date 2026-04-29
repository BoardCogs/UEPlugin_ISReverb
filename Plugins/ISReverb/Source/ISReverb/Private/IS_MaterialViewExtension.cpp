#include "IS_MaterialViewExtension.h"
#include "Runtime/Renderer/Internal/PostProcess/PostProcessInputs.h"
#include "Runtime/Renderer/Private/ScenePrivate.h"
#include "Runtime/Renderer/Private/SceneRendering.h"
#include "SceneTextureParameters.h"
#include "RayTracingShaderBindingLayout.h"
#include "Nanite/NaniteRayTracing.h"
#include "RayTracing/RayTracingMaterialHitShaders.h"
#include "MaterialShaderType.h"
#include "RHIResources.h"

static TAutoConsoleVariable<int32> CVarMaterialView(
	TEXT("r.Raytracing.MaterialView.Enable"),
	0,
	TEXT("Enables material properties visualization: all meshes will be rendered with a simple color showcasing roughness, metallic and specular factor.\n"
		      "Red = Roughness\n"
		      "Green = Metallic\n"
		      "Blue = Specular\n"
			  "Note: Crash if ray tracing shadows are not enabled.\n"
			  "0: Off, 1: On"),
	ECVF_RenderThreadSafe
);

static TAutoConsoleVariable<int32> CVarMaterialViewDiffuse(
	TEXT("r.Raytracing.MaterialViewDiffuse.Enable"),
	1,
	TEXT("Enables a simple diffuse lighting for the material view.\n"
			  "0: Off, 1: On"),
	ECVF_RenderThreadSafe
);

static TAutoConsoleVariable<int32> CVarMaterialViewAmbientOcclusion(
	TEXT("r.Raytracing.MaterialViewAmbientOcclusion.Enable"),
	0,
	TEXT("Enables ambient occlusion for the material view.\n"
			  "0: Off, 1: On"),
	ECVF_RenderThreadSafe
);

static TAutoConsoleVariable<int32> CVarMaterialViewShadow(
	TEXT("r.Raytracing.MaterialViewShadow.Enable"),
	0,
	TEXT("Enables simple shadows (only for directional lights) for the material view.\n"
			  "0: Off, 1: On"),
	ECVF_RenderThreadSafe
);



#if RHI_RAYTRACING

class FMaterialViewRG : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FMaterialViewRG)
	SHADER_USE_ROOT_PARAMETER_STRUCT(FMaterialViewRG, FGlobalShader)

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_SCALAR_ARRAY(int, RenderParams, [3])
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutputTexture)
		SHADER_PARAMETER_STRUCT_INCLUDE(FSceneTextureShaderParameters, SceneTextures)
		SHADER_PARAMETER_RDG_UNIFORM_BUFFER(FSceneUniformParameters, Scene)
		SHADER_PARAMETER_RDG_UNIFORM_BUFFER(FNaniteRayTracingUniformParameters, NaniteRayTracing)
		SHADER_PARAMETER_RDG_BUFFER_SRV(RaytracingAccelerationStructure, TLAS)
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, ViewUniformBuffer)
	END_SHADER_PARAMETER_STRUCT()
	
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("RAY_TRACING_PAYLOAD_TYPE"), 0);
	}
	
	static ERayTracingPayloadType GetRayTracingPayloadType(const int32 /*PermutationId*/)
	{
		return ERayTracingPayloadType::RayTracingMaterial;
	}
	static const FShaderBindingLayout* GetShaderBindingLayout(const FShaderPermutationParameters& Parameters)
	{
		return RayTracing::GetShaderBindingLayout(Parameters.Platform);
	}
};
IMPLEMENT_GLOBAL_SHADER(FMaterialViewRG, "/Plugin/ISReverb/MaterialView.usf", "MaterialViewRG", SF_RayGen);

#endif



FIS_MaterialViewExtension::FIS_MaterialViewExtension(const FAutoRegister& AutoRegister)
	: FSceneViewExtensionBase(AutoRegister)
{
}

void FIS_MaterialViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& InView,
	const FPostProcessingInputs& Inputs)
{
	if (CVarMaterialView.GetValueOnRenderThread())
	{
		FScene* Scene = InView.Family->Scene->GetRenderScene();
		if (!Scene || !InView.IsRayTracingAllowedForView()) return;
		
		const FViewInfo& View = static_cast<const FViewInfo&>(InView);
		const FRayTracingScene& RayTracingScene = Scene->RayTracingScene;

		if (!RayTracingScene.IsCreated()) return;
		
		const FIntRect PrimaryViewRect = View.ViewRect;
		FScreenPassTexture SceneColor((*Inputs.SceneTextures)->SceneColorTexture, PrimaryViewRect);

		//出力用のテクスチャを作成
		FIntPoint TextureSize = SceneColor.Texture->Desc.Extent;
		FRDGTextureDesc OutputDesc = FRDGTextureDesc::Create2D(
			TextureSize,
			SceneColor.Texture->Desc.Format,
			FClearValueBinding::None,
			TexCreate_ShaderResource | TexCreate_UAV
		);

		FRDGTexture* OutputRDGTexture = GraphBuilder.CreateTexture(OutputDesc, TEXT("MaterialView.Output"));
		FRDGTextureUAV* OutputUAV = GraphBuilder.CreateUAV(OutputRDGTexture);
		
		//pass param
		{

			TShaderMapRef<FMaterialViewRG> RayGenerationShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
			FMaterialViewRG::FParameters* PassParameters = GraphBuilder.AllocParameters<FMaterialViewRG::FParameters>();
			
			PassParameters->ViewUniformBuffer = InView.ViewUniformBuffer;
			PassParameters->TLAS = RayTracingScene.GetLayerView(ERayTracingSceneLayer::Base);
			PassParameters->Scene = GetSceneUniformBufferRef(GraphBuilder, InView);
			PassParameters->OutputTexture = OutputUAV;
			PassParameters->NaniteRayTracing = Nanite::GetPublicGlobalRayTracingUniformBuffer();
			FSceneTextures SceneTextures = View.GetSceneTextures();
			PassParameters->SceneTextures = GetSceneTextureShaderParameters(Inputs.SceneTextures);
			GET_SCALAR_ARRAY_ELEMENT(PassParameters->RenderParams, 0) = CVarMaterialViewDiffuse.GetValueOnRenderThread();
			GET_SCALAR_ARRAY_ELEMENT(PassParameters->RenderParams, 1) = CVarMaterialViewAmbientOcclusion.GetValueOnRenderThread();
			GET_SCALAR_ARRAY_ELEMENT(PassParameters->RenderParams, 2) = CVarMaterialViewShadow.GetValueOnRenderThread();
			
			GraphBuilder.AddPass(
	        RDG_EVENT_NAME("MaterialViewRG"),
	        PassParameters,
	        ERDGPassFlags::Compute| ERDGPassFlags::NeverCull,
	        [PassParameters, RayGenerationShader, TextureSize, &View](FRHICommandList& RHICmdList)
	        {
        		if (!View.MaterialRayTracingData.PipelineState) return;
	        	
	        	//シェーダーのバインドをルートシグネチャと合わせるために必要
        		FRHIUniformBuffer* SceneUniformBuffer = PassParameters->Scene->GetRHI();
				FRHIUniformBuffer* NaniteRayTracingUniformBuffer = PassParameters->NaniteRayTracing->GetRHI();
				TOptional<FScopedUniformBufferStaticBindings> StaticUniformBufferScope = RayTracing::BindStaticUniformBufferBindings(View, SceneUniformBuffer, NaniteRayTracingUniformBuffer, RHICmdList);
        		
	        	//エンジン組み込みのパイプラインを使用する
        		FRayTracingPipelineState* PipeLine =View.MaterialRayTracingData.PipelineState; 
				FShaderBindingTableRHIRef SBT = View.MaterialRayTracingData.ShaderBindingTable;
	        	
	            FRHIBatchedShaderParameters& GlobalResources = RHICmdList.GetScratchShaderParameters();
	            SetShaderParameters(GlobalResources, RayGenerationShader, *PassParameters);
        		
				RHICmdList.RayTraceDispatch(
					PipeLine,
					RayGenerationShader.GetRayTracingShader(),
					SBT,
					GlobalResources, 
					TextureSize.X, TextureSize.Y
				);
	        }
		);
		}

		AddCopyTexturePass(GraphBuilder, OutputRDGTexture, SceneColor.Texture);
	}
}

void FIS_MaterialViewExtension::OnPrepareRayTracing(const class FViewInfo& View,
	TArray<FRHIRayTracingShader*>& OutRayGenShaders)
{
	TShaderMapRef<FMaterialViewRG> RayGenerationShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	OutRayGenShaders.Add(RayGenerationShader.GetRayTracingShader());
}

/*
//リンカーエラーが出るためSceneRendering.cppから関数をコピー
const FViewInfo* FViewInfo::GetPrimaryView() const
{
	// It is valid for this function to return itself if it's already the primary view.
	if (Family && Family->Views.IsValidIndex(PrimaryViewIndex))
	{
		const FSceneView* PrimaryView = Family->Views[PrimaryViewIndex];
		check(PrimaryView->bIsViewInfo);
		return static_cast<const FViewInfo*>(PrimaryView);
	}
	return this;
}
*/