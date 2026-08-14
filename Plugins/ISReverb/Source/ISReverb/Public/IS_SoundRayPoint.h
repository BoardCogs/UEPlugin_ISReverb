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

	// Sound levels (in percentage) right before sound bounces off surface, for three different frequency bands
	// These values are not present in the first point of each ray, as it indicates the source
	float InLevel125;
	float InLevel250;
	float InLevel500;
	float InLevel1000;
	float InLevel2000;
	float InLevel4000;
	
	// Sound levels (in percentage) right after sound bounces off surface, for three different frequency bands
	// These values are not present in the last point of each ray, as it indicates the listener
	float OutLevel125;
	float OutLevel250;
	float OutLevel500;
	float OutLevel1000;
	float OutLevel2000;
	float OutLevel4000;
	
};
