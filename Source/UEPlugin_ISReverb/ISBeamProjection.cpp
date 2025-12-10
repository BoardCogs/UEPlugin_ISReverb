#include "ISBeamProjection.h"

ISBeamProjection::ISBeamProjection(TArray<FVector3f> points, TArray<ReflectorEdge> edges)
{
	_points = TArray(points);
	_edges = TArray(edges);
}



// Returns all points of the surface polygon
TArray<FVector3f> ISBeamProjection::Points()
{
	return _points;
}

// Returns all edges of the surface polygon
TArray<ReflectorEdge> ISBeamProjection::Edges()
{
	return _edges;
}



// Removes a point from the projection polygon
void ISBeamProjection::RemovePoint(FVector3f point)
{
	_points.Remove(point);
}

// Add a point to the projection polygon
void ISBeamProjection::AddPoint(FVector3f point)
{
	if (!_points.Contains(point))
		_points.Add(point);
}

// Removes an edge from the projection polygon
void ISBeamProjection::RemoveEdge(ReflectorEdge edge)
{
	_edges.Remove(edge);
}

// Add an edge to the projection polygon
ReflectorEdge ISBeamProjection::AddEdge(FVector3f pointA, FVector3f pointB)
{
	ReflectorEdge edge = ReflectorEdge(pointA, pointB);

	if (_edges.Contains(edge) || pointA == pointB)
	{
		return ReflectorEdge::Void();
	}

	_edges.Add( edge );

	return edge;
}



// Given a and b, two points connected by an edge, finds the other edge that a belongs to
ReflectorEdge ISBeamProjection::FindOtherEdge(FVector3f a, FVector3f b)
{
	for (ReflectorEdge edge : _edges)
	{
		// If one extreme of the edge is a and the other is not b, then return the other edge
		if ( ( edge.PointA == a || edge.PointB == a ) && edge.PointB != b && edge.PointA != b )
		{
			return edge;
		}
	}

	return ReflectorEdge::Void();
}



ISBeamProjection::~ISBeamProjection()
{
}
