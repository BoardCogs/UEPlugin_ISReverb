#pragma once

#include "CoreMinimal.h"
#include "IS_SoundRay.h"

/**
 * 
 */
class ISREVERB_API IS_SoundRayArray
{

public:
	// Methods
	
	IS_SoundRayArray();
	
	void Empty();
	
	void AddRay(IS_SoundRay Ray);
	
	int GetNumRays();

	IS_SoundRay* GetRay(int i);

	// Properties

	// Contains all sound rays that succesfully reach the listener 
	TArray<IS_SoundRay> SoundRays;
	
};
