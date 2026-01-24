#pragma once

#include "TmpIS_ReflectorEdge.h"
#include "CoreMinimal.h"



/**
 * Projection of the father IS's beam onto the surface of a lower order IS.
 * Each projection is represented by a convex polygon, defined by its points and edges.
 */
class UEPLUGIN_ISREVERB_API TmpIS_BeamProjection
{
public:
	// CONSTRUCTOR
	TmpIS_BeamProjection(TArray<FVector3f> points, TArray<TmpIS_ReflectorEdge> edges);

	// METHODS
	
	TArray<FVector3f> Points();
	
	TArray<TmpIS_ReflectorEdge> Edges();

	void RemovePoint(FVector3f point);

	void AddPoint(FVector3f point);

	void RemoveEdge(TmpIS_ReflectorEdge edge);

	TmpIS_ReflectorEdge AddEdge(FVector3f pointA, FVector3f pointB);

	// Given a and b, two points connected by an edge, finds the other edge that a belongs to
	TmpIS_ReflectorEdge FindOtherEdge(FVector3f a, FVector3f b);

	~TmpIS_BeamProjection();
	
private:
	// PROPERTIES
	TArray<FVector3f> _points;
	
	TArray<TmpIS_ReflectorEdge> _edges;
};
