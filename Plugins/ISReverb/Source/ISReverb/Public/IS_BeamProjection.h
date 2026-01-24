#pragma once

#include "IS_ReflectorEdge.h"
#include "CoreMinimal.h"



/**
 * Projection of the father IS's beam onto the surface of a lower order IS.
 * Each projection is represented by a convex polygon, defined by its points and edges.
 */
class ISREVERB_API IS_BeamProjection
{
public:
	// CONSTRUCTOR
	IS_BeamProjection(TArray<FVector3f> points, TArray<IS_ReflectorEdge> edges);

	// METHODS
	
	TArray<FVector3f> Points();
	
	TArray<IS_ReflectorEdge> Edges();

	void RemovePoint(FVector3f point);

	void AddPoint(FVector3f point);

	void RemoveEdge(IS_ReflectorEdge edge);

	IS_ReflectorEdge AddEdge(FVector3f pointA, FVector3f pointB);

	// Given a and b, two points connected by an edge, finds the other edge that a belongs to
	IS_ReflectorEdge FindOtherEdge(FVector3f a, FVector3f b);

	~IS_BeamProjection();
	
private:
	// PROPERTIES
	TArray<FVector3f> _points;
	
	TArray<IS_ReflectorEdge> _edges;
};
