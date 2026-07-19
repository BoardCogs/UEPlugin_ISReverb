#include "IS_SoundRayPoint.h"

IS_SoundRayPoint::IS_SoundRayPoint()
{
	
}

IS_SoundRayPoint::IS_SoundRayPoint(FVector3f position, float inAmp125, float outAmp125, float inAmp250, float outAmp250, float inAmp500, float outAmp500, float inAmp1000, float outAmp1000, float inAmp2000, float outAmp2000, float inAmp4000, float outAmp4000)
{
	PointPosition = position;
	InAmplitude125 = inAmp125;
	OutAmplitude125 = outAmp125;
	InAmplitude250 = inAmp250;
	OutAmplitude250 = outAmp250;
	InAmplitude500 = inAmp500;
	OutAmplitude500 = outAmp500;
	InAmplitude1000 = inAmp1000;
	OutAmplitude1000 = outAmp1000;
	InAmplitude2000 = inAmp2000;
	OutAmplitude2000 = outAmp2000;
	InAmplitude4000 = inAmp4000;
	OutAmplitude4000 = outAmp4000;
}