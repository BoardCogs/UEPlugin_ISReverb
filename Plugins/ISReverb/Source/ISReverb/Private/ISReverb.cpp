#include "ISReverb.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "IS_MaterialViewExtension.h"
#include "ShaderCore.h"
#include "DeferredShadingRenderer.h" 

#define LOCTEXT_NAMESPACE "FISReverbModule"

void FISReverbModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/ISReverb"), FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("ISReverb"))->GetBaseDir(), TEXT("Shaders/Private")));
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FISReverbModule::OnPostEngineInit);
	PrepareRayTracingDelegateHandle = FGlobalIlluminationPluginDelegates::PrepareRayTracing().AddRaw(this, &FISReverbModule::OnPrepareRayTracing);
}

void FISReverbModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	ViewExtension.Reset();
}

void FISReverbModule::OnPostEngineInit()
{
	ViewExtension = FSceneViewExtensions::NewExtension<FIS_MaterialViewExtension>();
}

void FISReverbModule::OnPrepareRayTracing(const class FViewInfo& View,
	TArray<FRHIRayTracingShader*>& OutRayGenShaders)
{
	ViewExtension->OnPrepareRayTracing(View, OutRayGenShaders);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FISReverbModule, ISReverb)