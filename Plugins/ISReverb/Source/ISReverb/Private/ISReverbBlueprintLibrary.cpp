// Copyright Epic Games, Inc. All Rights Reserved.

#include "ISReverbBlueprintLibrary.h"



PRAGMA_DISABLE_DEPRECATION_WARNINGS
UISReverbBlueprintLibrary::UISReverbBlueprintLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{ }


// static
void UISReverbBlueprintLibrary::GetUndistortOverscanFactor(
	const FISReverbCameraModel& CameraModel,
	float DistortedHorizontalFOV,
	float DistortedAspectRatio,
	float& UndistortOverscanFactor)
{
	UndistortOverscanFactor = CameraModel.GetUndistortOverscanFactor(DistortedHorizontalFOV, DistortedAspectRatio);
}


// static
void UISReverbBlueprintLibrary::DrawUVDisplacementToRenderTarget(
	const UObject* WorldContextObject,
	const FISReverbCameraModel& CameraModel,
	float DistortedHorizontalFOV,
	float DistortedAspectRatio,
	float UndistortOverscanFactor,
	class UTextureRenderTarget2D* OutputRenderTarget,
	float OutputMultiply,
	float OutputAdd)
{
	CameraModel.DrawUVDisplacementToRenderTarget(
		WorldContextObject->GetWorld(),
		DistortedHorizontalFOV, DistortedAspectRatio,
		UndistortOverscanFactor, OutputRenderTarget,
		OutputMultiply, OutputAdd);
}
PRAGMA_ENABLE_DEPRECATION_WARNINGS
