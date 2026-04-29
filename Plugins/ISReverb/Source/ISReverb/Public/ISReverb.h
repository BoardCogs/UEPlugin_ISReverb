#pragma once

#include "Modules/ModuleManager.h"

class FIS_MaterialViewExtension;
class FRHIRayTracingShader;
class FISReverbModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	
	void OnPostEngineInit();
	TSharedPtr<FIS_MaterialViewExtension, ESPMode::ThreadSafe> ViewExtension;
	void OnPrepareRayTracing(const class FViewInfo& View, TArray<FRHIRayTracingShader*>& OutRayGenShaders);
	FDelegateHandle PrepareRayTracingDelegateHandle;
};
