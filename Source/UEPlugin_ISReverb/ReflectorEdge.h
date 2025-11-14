// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class UEPLUGIN_ISREVERB_API ReflectorEdge
{
public:
	FVector3d PointA;
	FVector3d PointB;
	
	ReflectorEdge(FVector3d a, FVector3d b);

	FVector3d Direction();
	
	double Length();
	
	~ReflectorEdge();
};
