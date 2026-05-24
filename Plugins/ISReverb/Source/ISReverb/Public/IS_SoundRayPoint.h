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
	
	IS_SoundRayPoint(FVector3f position, float inAmp, float outAmp);

	// Properties

	// The world position of this point
	FVector3f PointPosition;

	// Sound amplitude right before sound bounces off surface
	// This value is not present in the first point of each ray, as it indicates the source
	float InAmplitude;
	
	// Sound amplitude right after sound bounces off surface
	// This value is not present in the last point of each ray, as it indicates the listener
	float OutAmplitude;
	
};
