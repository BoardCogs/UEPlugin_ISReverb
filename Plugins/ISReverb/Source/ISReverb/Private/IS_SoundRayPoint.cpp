#include "IS_SoundRayPoint.h"

IS_SoundRayPoint::IS_SoundRayPoint()
{
	
}

IS_SoundRayPoint::IS_SoundRayPoint(FVector3f position, float inAmpLow, float outAmpLow, float inAmpMed, float outAmpMed, float inAmpHigh, float outAmpHigh)
{
	PointPosition = position;
	InAmplitudeLow = inAmpLow;
	OutAmplitudeLow = outAmpLow;
	InAmplitudeMed = inAmpMed;
	OutAmplitudeMed = outAmpMed;
	InAmplitudeHigh = inAmpHigh;
	OutAmplitudeHigh = outAmpHigh;
}