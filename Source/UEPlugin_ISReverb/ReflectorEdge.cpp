#include "ReflectorEdge.h"

ReflectorEdge::ReflectorEdge(FVector3f a, FVector3f b)
{
	PointA = a;
	PointB = b;
}

// Returns unit vector expressing the direction followed by the edge from pointA to pointB
FVector3f ReflectorEdge::Direction()
{
	return (PointA - PointB).GetSafeNormal();
}

// Returns length of the edge in units
float ReflectorEdge::Length()
{
	return (PointB - PointA).Length();
}

ReflectorEdge::~ReflectorEdge()
{
}
