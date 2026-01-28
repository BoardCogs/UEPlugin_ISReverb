#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ISReverbAPI.h"
#include "ISReverbBlueprintLibrary.generated.h"



UCLASS(MinimalAPI, meta=(ScriptName="ISReverbLibrary"))
class UISReverbBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()


	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	/** Returns the overscan factor required for the undistort rendering to avoid unrendered distorted pixels. */
	UFUNCTION(BlueprintPure, Category = "ISReverb | Lens Distortion")
	static void GetUndistortOverscanFactor(
		const FISReverbCameraModel& CameraModel,
		float DistortedHorizontalFOV,
		float DistortedAspectRatio,
		float& UndistortOverscanFactor);
		
	/** Draws UV displacement map within the output render target.
	 * - Red & green channels hold the distortion displacement;
	 * - Blue & alpha channels hold the undistortion displacement.
	 * @param DistortedHorizontalFOV The desired horizontal FOV in the distorted render.
	 * @param DistortedAspectRatio The desired aspect ratio of the distorted render.
	 * @param UndistortOverscanFactor The factor of the overscan for the undistorted render.
	 * @param OutputRenderTarget The render target to draw to. Don't necessarily need to have same resolution or aspect ratio as distorted render.
	 * @param OutputMultiply The multiplication factor applied on the displacement.
	 * @param OutputAdd Value added to the multiplied displacement before storing into the output render target.
	 */
	UFUNCTION(BlueprintCallable, Category = "ISReverb | Lens Distortion", meta = (WorldContext = "WorldContextObject"))
	static void DrawUVDisplacementToRenderTarget(
		const UObject* WorldContextObject,
		const FISReverbCameraModel& CameraModel,
		float DistortedHorizontalFOV,
		float DistortedAspectRatio,
		float UndistortOverscanFactor,
		class UTextureRenderTarget2D* OutputRenderTarget,
		float OutputMultiply = 0.5,
		float OutputAdd = 0.5
		);

	/* Returns true if A is equal to B (A == B) */
	UFUNCTION(BlueprintPure, meta=(DisplayName = "Equal (ISReverbCameraModel)", CompactNodeTitle = "==", Keywords = "== equal"), Category = "ISReverb | Lens Distortion")
	static bool EqualEqual_CompareISReverbModels(
		const FISReverbCameraModel& A,
		const FISReverbCameraModel& B)
	{
		return A == B;
	}

	/* Returns true if A is not equal to B (A != B) */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "NotEqual (ISReverbCameraModel)", CompactNodeTitle = "!=", Keywords = "!= not equal"), Category = "ISReverb | Lens Distortion")
	static bool NotEqual_CompareISReverbModels(
		const FISReverbCameraModel& A,
		const FISReverbCameraModel& B)
	{
		return A != B;
	}
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

};
