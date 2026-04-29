#pragma once

#include "CoreMinimal.h"
#include "SceneViewExtension.h"

class ISREVERB_API FIS_MaterialViewExtension:public FSceneViewExtensionBase
{
public:
	FIS_MaterialViewExtension(const FAutoRegister& AutoRegister);
	~FIS_MaterialViewExtension() = default;
	
	virtual void PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& InView, const FPostProcessingInputs& Inputs) override;
	void OnPrepareRayTracing(const class FViewInfo& View, TArray<FRHIRayTracingShader*>& OutRayGenShaders);
};
