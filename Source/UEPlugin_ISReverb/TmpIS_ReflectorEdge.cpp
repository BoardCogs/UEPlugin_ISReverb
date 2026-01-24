#include "TmpIS_ReflectorEdge.h"

// Edge constructor
TmpIS_ReflectorEdge::TmpIS_ReflectorEdge(FVector3f a, FVector3f b)
{
	PointA = a;
	PointB = b;
}



// Returns unit vector expressing the direction followed by the edge from pointA to pointB
FVector3f TmpIS_ReflectorEdge::Direction()
{
	return (PointA - PointB).GetSafeNormal();
}

// Returns length of the edge in units
float TmpIS_ReflectorEdge::Length()
{
	return (PointB - PointA).Length();
}



// Returns an empty edge
TmpIS_ReflectorEdge TmpIS_ReflectorEdge::Void()
{
	return TmpIS_ReflectorEdge(FVector3f::Zero(), FVector3f::Zero());
}

// Returns true if the edge has zero vectors for both extremes
bool TmpIS_ReflectorEdge::IsVoid()
{
	return PointA == FVector3f::Zero() && PointB == FVector3f::Zero();
}

// Equal-to operator
bool operator==(const TmpIS_ReflectorEdge& lhs, const TmpIS_ReflectorEdge& rhs)
{
	return lhs.PointA == rhs.PointA && lhs.PointB == rhs.PointB;
}



TmpIS_ReflectorEdge::~TmpIS_ReflectorEdge()
{
}