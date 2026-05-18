// Fill out your copyright notice in the Description page of Project Settings.


#include "IS_SoundRay.h"

UIS_SoundRay::UIS_SoundRay()
{
	RayPoints = TArray<UIS_SoundRayPoint>();
}

int UIS_SoundRay::GetNumRayPoints()
{
	return RayPoints.Num();
}

UIS_SoundRayPoint* UIS_SoundRay::GetRayPoint(int i)
{
	return &RayPoints[i];
}