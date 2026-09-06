#include "IS_SoundRayPoint.h"

IS_SoundRayPoint::IS_SoundRayPoint()
{
	
}

IS_SoundRayPoint::IS_SoundRayPoint(FVector3f position, float inAmp125, float outAmp125, float inAmp250, float outAmp250, float inAmp500, float outAmp500, float inAmp1000, float outAmp1000, float inAmp2000, float outAmp2000, float inAmp4000, float outAmp4000)
{
	PointPosition = position;
	InLevel125 = inAmp125;
	OutLevel125 = outAmp125;
	InLevel250 = inAmp250;
	OutLevel250 = outAmp250;
	InLevel500 = inAmp500;
	OutLevel500 = outAmp500;
	InLevel1000 = inAmp1000;
	OutLevel1000 = outAmp1000;
	InLevel2000 = inAmp2000;
	OutLevel2000 = outAmp2000;
	InLevel4000 = inAmp4000;
	OutLevel4000 = outAmp4000;
}