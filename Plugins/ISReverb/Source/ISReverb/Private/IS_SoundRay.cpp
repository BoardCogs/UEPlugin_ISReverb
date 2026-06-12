#include "IS_SoundRay.h"

IS_SoundRay::IS_SoundRay()
{
	RayPoints = TArray<IS_SoundRayPoint>();
}

void IS_SoundRay::Clear()
{
	RayPoints.Empty();
	FinalAmplitudes = FVector3f().ZeroVector;
	ISPosition = FVector3f(0,0,0);
}

void IS_SoundRay::AddRayPoint(IS_SoundRayPoint point)
{
	RayPoints.Add(point);
}

int IS_SoundRay::GetNumRayPoints()
{
	return RayPoints.Num();
}

IS_SoundRayPoint* IS_SoundRay::GetRayPoint(int i)
{
	return &RayPoints[i];
}