// Fill out your copyright notice in the Description page of Project Settings.


#include "ReflectorEdge.h"

ReflectorEdge::ReflectorEdge(FVector3d a, FVector3d b)
{
	PointA = a;
	PointB = b;
}

FVector3d ReflectorEdge::Direction()
{
	return (PointA - PointB).GetSafeNormal();
}
	
double ReflectorEdge::Length()
{
	return (PointB - PointA).Length();
}

ReflectorEdge::~ReflectorEdge()
{
}
