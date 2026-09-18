#include "IS_SoundRayArray.h"

IS_SoundRayArray::IS_SoundRayArray()
{
	SoundRays = TArray<IS_SoundRay>();
}

void IS_SoundRayArray::Empty()
{
	SoundRays.Empty();
}

int IS_SoundRayArray::GetNumRays()
{
	return SoundRays.Num();
}

IS_SoundRay* IS_SoundRayArray::GetRay(int i)
{
	return &SoundRays[i];
}