// Copyright Epic Games, Inc. All Rights Reserved.

#include "ISReverb.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FISReverbModule"

void FISReverbModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/ISReverb"), FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("ISReverb"))->GetBaseDir(), TEXT("Shaders/Private")));
}

void FISReverbModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FISReverbModule, ISReverb)