#pragma once

#include "CoreMinimal.h"
#include "IS_SoundRayPoint.h"

/**
 * 
 */
class ISREVERB_API IS_SoundRay
{
	
public:
	// Methods
	
	IS_SoundRay();

	void Clear();

	void AddRayPoint(IS_SoundRayPoint point);

	int GetNumRayPoints();
	
	IS_SoundRayPoint* GetRayPoint(int i);

	// Properties

	// All points along which the sound ray gets reflected, plus source and listener as first and last point respectively
	TArray<IS_SoundRayPoint> RayPoints;

	// The final amplitude of the reverb reaching the listener
	float FinalAmplitude;

	// Position of the IS generating this ray, it corresponds to the position from which the reverb appears to be heard
	FVector3f ISPosition;
	
};
