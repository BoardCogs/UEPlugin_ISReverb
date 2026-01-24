#include "IS_ReflectorEdge.h"

// Edge constructor
IS_ReflectorEdge::IS_ReflectorEdge(FVector3f a, FVector3f b)
{
	PointA = a;
	PointB = b;
}



// Returns unit vector expressing the direction followed by the edge from pointA to pointB
FVector3f IS_ReflectorEdge::Direction()
{
	return (PointA - PointB).GetSafeNormal();
}

// Returns length of the edge in units
float IS_ReflectorEdge::Length()
{
	return (PointB - PointA).Length();
}



// Returns an empty edge
IS_ReflectorEdge IS_ReflectorEdge::Void()
{
	return IS_ReflectorEdge(FVector3f::Zero(), FVector3f::Zero());
}

// Returns true if the edge has zero vectors for both extremes
bool IS_ReflectorEdge::IsVoid()
{
	return PointA == FVector3f::Zero() && PointB == FVector3f::Zero();
}

// Equal-to operator
bool operator==(const IS_ReflectorEdge& lhs, const IS_ReflectorEdge& rhs)
{
	return lhs.PointA == rhs.PointA && lhs.PointB == rhs.PointB;
}



IS_ReflectorEdge::~IS_ReflectorEdge()
{
}