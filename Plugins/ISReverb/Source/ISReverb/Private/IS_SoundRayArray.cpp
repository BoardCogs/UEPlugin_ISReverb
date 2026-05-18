// Fill out your copyright notice in the Description page of Project Settings.


#include "IS_SoundRayArray.h"

UIS_SoundRayArray::UIS_SoundRayArray()
{
	SoundRays = TArray<UIS_SoundRay>();
}

int UIS_SoundRayArray::GetNumRays()
{
	return SoundRays.Num();
}

UIS_SoundRay* UIS_SoundRayArray::GetRay(int i)
{
	return &SoundRays[i];
}