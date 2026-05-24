#include "IS_SoundRayPoint.h"

IS_SoundRayPoint::IS_SoundRayPoint()
{
	
}

IS_SoundRayPoint::IS_SoundRayPoint(FVector3f position, float inAmp, float outAmp)
{
	PointPosition = position;
	InAmplitude = inAmp;
	OutAmplitude = outAmp;
}