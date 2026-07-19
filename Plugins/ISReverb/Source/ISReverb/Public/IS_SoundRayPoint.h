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
	
	IS_SoundRayPoint(FVector3f position, float inAmp125, float outAmp125, float inAmp250, float outAmp250, float inAmp500, float outAmp500, float inAmp1000, float outAmp1000, float inAmp2000, float outAmp2000, float inAmp4000, float outAmp4000);

	// Properties

	// The world position of this point
	FVector3f PointPosition;

	// Sound amplitudes right before sound bounces off surface, for three different frequency bands
	// These values are not present in the first point of each ray, as it indicates the source
	float InAmplitude125;
	float InAmplitude250;
	float InAmplitude500;
	float InAmplitude1000;
	float InAmplitude2000;
	float InAmplitude4000;
	
	// Sound amplitudes right after sound bounces off surface, for three different frequency bands
	// These values are not present in the last point of each ray, as it indicates the listener
	float OutAmplitude125;
	float OutAmplitude250;
	float OutAmplitude500;
	float OutAmplitude1000;
	float OutAmplitude2000;
	float OutAmplitude4000;
	
};
