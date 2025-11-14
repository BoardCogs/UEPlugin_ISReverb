// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class UEPLUGIN_ISREVERB_API ReflectorEdge
{
public:
	FVector3f PointA;
	FVector3f PointB;
	
	ReflectorEdge(FVector3f a, FVector3f b);

	FVector3f Direction();
	
	double Length();
	
	~ReflectorEdge();
	
};
