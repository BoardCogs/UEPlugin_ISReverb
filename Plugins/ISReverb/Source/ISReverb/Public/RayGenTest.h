#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderTargetPool.h"
#include "RHI.h"
#include "RHIResources.h"
#include "Modules/ModuleManager.h"
#include "RayTracingDefinitions.h"
#include "RayTracingPayloadType.h"
#include "../Private/RayTracing/RayTracingScene.h"
#include "../Private/SceneRendering.h"
#include "RenderGraphUtils.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"



class FRayTracingScene;

struct FRayGenTestParameters
{

	FIntPoint GetRenderTargetSize() const
	{
		return CachedRenderTargetSize;
	}

	FRayGenTestParameters() {};
	
	FRayGenTestParameters(UTextureRenderTarget2D* IORenderTarget)
		: RenderTarget(IORenderTarget)
	{
		CachedRenderTargetSize = RenderTarget ? FIntPoint(RenderTarget->SizeX, RenderTarget->SizeY) : FIntPoint::ZeroValue;
	}

	UTextureRenderTarget2D* RenderTarget;
	FIntPoint CachedRenderTargetSize;
};

class ISREVERB_API FRayGenTest
{
public:
	FRayGenTest();

	void BeginRendering();
	void EndRendering();
	void UpdateParameters(FRayGenTestParameters& DrawParameters);
private:
	void Execute_RenderThread(FPostOpaqueRenderParameters& Parameters);
	
	/// The delegate handle to our function that will be executed each frame by the renderer
	FDelegateHandle PostOpaqueRenderDelegate;
	/// Cached Shader Manager Parameters
	FRayGenTestParameters CachedParams;
	/// Whether we have cached parameters to pass to the shader or not
	volatile bool bCachedParamsAreValid;

	/// We create the shader's output texture and UAV and save to avoid reallocation
	FTextureRHIRef ShaderOutputTexture;
	FUnorderedAccessViewRHIRef ShaderOutputTextureUAV;
	bool TextureCreated;
};