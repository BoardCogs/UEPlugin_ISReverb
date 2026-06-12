#pragma once

#include "CoreMinimal.h"

/**
 * A point in space where a sound ray hits a surface and is then reflected
 * The first point of a ray identifies the source, while the last is the listener
 */
class ISREVERB_API IS_SoundRayPoint
{

public:
	// Methods
	
	IS_SoundRayPoint();
	
	IS_SoundRayPoint(FVector3f position, float inAmpLow, float outAmpLow, float inAmpMed, float outAmpMed, float inAmpHigh, float outAmpHigh);

	// Properties

	// The world position of this point
	FVector3f PointPosition;

	// Sound amplitudes right before sound bounces off surface, for three different frequency bands
	// These values are not present in the first point of each ray, as it indicates the source
	float InAmplitudeLow;
	float InAmplitudeMed;
	float InAmplitudeHigh;
	
	// Sound amplitudes right after sound bounces off surface, for three different frequency bands
	// These values are not present in the last point of each ray, as it indicates the listener
	float OutAmplitudeLow;
	float OutAmplitudeMed;
	float OutAmplitudeHigh;
	
};
