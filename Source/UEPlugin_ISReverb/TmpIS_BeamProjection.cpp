#include "TmpIS_BeamProjection.h"

TmpIS_BeamProjection::TmpIS_BeamProjection(TArray<FVector3f> points, TArray<TmpIS_ReflectorEdge> edges)
{
	_points = TArray(points);
	_edges = TArray(edges);
}



// Returns all points of the surface polygon
TArray<FVector3f> TmpIS_BeamProjection::Points()
{
	return _points;
}

// Returns all edges of the surface polygon
TArray<TmpIS_ReflectorEdge> TmpIS_BeamProjection::Edges()
{
	return _edges;
}



// Removes a point from the projection polygon
void TmpIS_BeamProjection::RemovePoint(FVector3f point)
{
	_points.Remove(point);
}

// Add a point to the projection polygon
void TmpIS_BeamProjection::AddPoint(FVector3f point)
{
	if (!_points.Contains(point))
		_points.Add(point);
}

// Removes an edge from the projection polygon
void TmpIS_BeamProjection::RemoveEdge(TmpIS_ReflectorEdge edge)
{
	_edges.Remove(edge);
}

// Add an edge to the projection polygon
TmpIS_ReflectorEdge TmpIS_BeamProjection::AddEdge(FVector3f pointA, FVector3f pointB)
{
	TmpIS_ReflectorEdge edge = TmpIS_ReflectorEdge(pointA, pointB);

	if (_edges.Contains(edge) || pointA == pointB)
	{
		return TmpIS_ReflectorEdge::Void();
	}

	_edges.Add( edge );

	return edge;
}



// Given a and b, two points connected by an edge, finds the other edge that a belongs to
TmpIS_ReflectorEdge TmpIS_BeamProjection::FindOtherEdge(FVector3f a, FVector3f b)
{
	for (TmpIS_ReflectorEdge edge : _edges)
	{
		// If one extreme of the edge is a and the other is not b, then return the other edge
		if ( ( edge.PointA == a || edge.PointB == a ) && edge.PointB != b && edge.PointA != b )
		{
			return edge;
		}
	}

	return TmpIS_ReflectorEdge::Void();
}



TmpIS_BeamProjection::~TmpIS_BeamProjection()
{
}
