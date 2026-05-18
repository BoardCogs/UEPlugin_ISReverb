// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IS_SoundRay.h"
#include "UObject/NoExportTypes.h"
#include "IS_SoundRayArray.generated.h"

/**
 * 
 */
UCLASS()
class ISREVERB_API UIS_SoundRayArray : public UObject
{
	GENERATED_BODY()

public:
	UIS_SoundRayArray();

	// Contains all sound rays that succesfully reach the listener 
	TArray<UIS_SoundRay> SoundRays;

	UFUNCTION(BlueprintCallable)
	int GetNumRays();

	UFUNCTION(BlueprintCallable)
	UIS_SoundRay* GetRay(int i);
	
};
