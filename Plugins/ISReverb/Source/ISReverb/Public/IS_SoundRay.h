#pragma once

#include "CoreMinimal.h"
#include "IS_SoundRayPoint.h"
#include "UObject/NoExportTypes.h"
#include "IS_SoundRay.generated.h"

/**
 * 
 */
UCLASS()
class ISREVERB_API UIS_SoundRay : public UObject
{
	GENERATED_BODY()

public:
	UIS_SoundRay();

	// All points along which the sound ray gets reflected, plus source and listener as first and last point respectively
	TArray<UIS_SoundRayPoint> RayPoints;

	UFUNCTION(BlueprintCallable)
	int GetNumRayPoints();

	UFUNCTION(BlueprintCallable)
	UIS_SoundRayPoint* GetRayPoint(int i);

	// The final amplitude of the reverb reaching the listener
	float FinalAmplitude;

	// Position of the IS generating this ray, it corresponds to the position from which the reverb appears to be heard
	FVector3f ISPosition;
	
};
