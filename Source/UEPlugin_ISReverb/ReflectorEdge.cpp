// Fill out your copyright notice in the Description page of Project Settings.


#include "ReflectorEdge.h"

ReflectorEdge::ReflectorEdge(FVector3d a, FVector3d b)
{
	PointA = a;
	PointB = b;
}

FVector3d ReflectorEdge::Direction()
{
	FVector3d direction = PointA - PointB;
	direction.Normalize();
	return direction;
}
	
double ReflectorEdge::Length()
{
	return (PointB - PointA).Length();
}

ReflectorEdge::~ReflectorEdge()
{
}
