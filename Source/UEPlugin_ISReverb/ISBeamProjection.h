#pragma once

#include "ReflectorEdge.h"
#include "CoreMinimal.h"

/**
 * Projection of the father IS's beam onto the surface of a lower order IS.
 * Each projection is represented by a convex polygon, defined by its points and edges.
 */
class UEPLUGIN_ISREVERB_API ISBeamProjection
{
public:
	ISBeamProjection(TArray<FVector3f> points, TArray<ReflectorEdge> edges);
	
	TArray<FVector3f> Points();
	
	TArray<ReflectorEdge> Edges();

	void RemovePoint(FVector3f point);

	void AddPoint(FVector3f point);

	void RemoveEdge(ReflectorEdge edge);

	ReflectorEdge AddEdge(FVector3f pointA, FVector3f pointB);

	// Given a and b, two points connected by an edge, finds the other edge that a belongs to
	ReflectorEdge FindOtherEdge(FVector3f a, FVector3f b);

	~ISBeamProjection();
	
private:
	TArray<FVector3f> _points;
	
	TArray<ReflectorEdge> _edges;
};
