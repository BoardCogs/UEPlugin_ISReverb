// Fill out your copyright notice in the Description page of Project Settings.


#include "IS_SoundRayPoint.h"

UIS_SoundRayPoint::UIS_SoundRayPoint()
{
	
}

UIS_SoundRayPoint::UIS_SoundRayPoint(FVector3f position, float inAmp, float outAmp)
{
	PointPosition = position;
	InAmplitude = inAmp;
	OutAmplitude = outAmp;
}