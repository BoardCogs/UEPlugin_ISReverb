#include "ReflectorEdge.h"

// Edge constructor
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



// Returns an empty edge
ReflectorEdge ReflectorEdge::Void()
{
	return ReflectorEdge(FVector3f::Zero(), FVector3f::Zero());
}

// Returns true if the edge has zero vectors for both extremes
bool ReflectorEdge::IsVoid()
{
	return PointA == FVector3f::Zero() && PointB == FVector3f::Zero();
}

// Equal-to operator
bool operator==(const ReflectorEdge& lhs, const ReflectorEdge& rhs)
{
	return lhs.PointA == rhs.PointA && lhs.PointB == rhs.PointB;
}



ReflectorEdge::~ReflectorEdge()
{
}