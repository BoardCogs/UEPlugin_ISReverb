// Fill out your copyright notice in the Description page of Project Settings.


#include "ReflectorEdge.h"

ReflectorEdge::ReflectorEdge(FVector3f a, FVector3f b)
{
	PointA = a;
	PointB = b;
}

FVector3f ReflectorEdge::Direction()
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
