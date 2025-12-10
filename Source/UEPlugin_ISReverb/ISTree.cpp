#include "ISTree.h"

#include <string>

ISTree::ISTree(int r, FVector3f sourcePos, TArray<ARoom*> rooms, bool wrongSideOfReflector, bool beamTracing, bool beamClipping, bool debugBeamTracing)
{
	if (r == 0)
        return;
    
    _ro = r;
    Rooms = rooms;
    _surfaces = Surfaces();
    _sn = _surfaces.Num();
    _wrongSideOfReflector = wrongSideOfReflector;
    _beamTracing = beamTracing;
    _beamClipping = beamClipping;
    _debugBeamTracing = debugBeamTracing;

    float timePassed = UGameplayStatics::GetTimeSeconds(Rooms[0]->GetWorld());

    TArray<int> firstNodeOfOrder = TArray{ 0, 0 };

    TArray<FVector3f> projectionPlanesNormals = TArray<FVector3f>();

    // Creating the first order ISs
    for (int i = 0; i < _sn ; i++, _realISs++)
    {
        _nodes.Add( IS( i, 1, -1, _surfaces[i], ISBeamProjection( _surfaces[i]->Points() , _surfaces[i]->Edges() ) ) );

        FVector3f pos = sourcePos;
            
        pos -= 2 * FVector3f::DotProduct(_surfaces[i]->Normal(),pos - _surfaces[i]->Origin()) * _surfaces[i]->Normal();

        _nodes[i].Position = pos;
    }
    
    // Creating all ISs from second order onward
    for (int i = _sn, order = 2 ; order <= _ro ; order++)
    {
        // Sets the first IS of the currently considered order of reflection
        firstNodeOfOrder.Add(i);

        // Checks on all ISs belonging to the previous order, acting as parents for new Image Sources
        for (int p = firstNodeOfOrder[order-1] ; p < firstNodeOfOrder[order] ; p++)
        {
            if (!_nodes[p].Valid)
                continue;

            // Beam projection planes for the parent are generated here, to avoid repeating the operation for each child
            projectionPlanesNormals = CreateProjectionPlanes( _nodes[p].Position, _nodes[p].BeamPoints );

            // Iterates on all surfaces, checking if a new IS can be derived from a reflection of the parent on them
            for (int s = 0 ; s < _sn ; s++)
            {
                if ( CreateIS(i, order, p, _surfaces[s], projectionPlanesNormals) )
                    i++;
            }
        }
    }

    timePassed = UGameplayStatics::GetTimeSeconds(Rooms[0]->GetWorld()) - timePassed;

    /*
    Sending to console a debug message showing the total number of ISs created and the amount saved by optimization.
    The number of not generated ISs is only the tip of the iceberg: their children would have also been generated.
    */
    UE_LOG(LogTemp, Display, TEXT("IS generation over in %f milliseconds\n"
                                  "Total number of ISs generated: %i\n"
                                  "Optimizations:\n"
                                  " - No reflection on same surface twice in a row: %i ISs removed\n"
                                  " - Wrong side of reflector: %i ISs removed\n"
                                  " - Beam tracing%hs: %i ISs removed"),
                                  timePassed * 1000, _realISs, _noDouble, _wrongSide, (_beamClipping ? " + clipping" : ""), _beam);
}



// This function checks all conditions for creating a new Image Source, then creates it if all are respected
bool ISTree::CreateIS(int i, int order, int parent, AReflectorSurface* surface, TArray<FVector3f> projectionPlanesNormals)
{
	// 1
    // Checking that no IS is created identifying a reflection on the same surface twice in a row
    // This is because a double reflection is impossible assuming flat surfaces
    if ( surface == _nodes[parent].Surface )
    {
        _noDouble++;
        return false;
    }

    // Computing the position of the new IS by mirroring its parent along the reflecting surface
    FVector3f pos = _nodes[parent].Position;
    pos -= 2 * FVector3f::DotProduct( surface->Normal() , pos - surface->Origin() ) * surface->Normal();



    // 2
    // Checking that the IS is not on the wrong side of the reflector, standing on the opposite side of the surface's normal
    if ( _wrongSideOfReflector && FVector3f::DotProduct( surface->Normal() , pos - surface->Origin() ) >= 0 )
    {
        _wrongSide++;
        return false;
    }



    // 3
    // Checking that reflections from parent to this surface are possible with beam tracing (+ clipping)

    ISBeamProjection beam = ISBeamProjection( surface->Points() , surface->Edges() );

    if (_beamTracing)
    {
        // Intersection with this edge
        FVector3f intersection;
        // Edge extreme that falls inside projection
        FVector3f inPoint;
        // Edge extreme that falls outside projection
        FVector3f outPoint;
        // The other edge connected to the outPoint
        ReflectorEdge otherEdge = ReflectorEdge(FVector3f::Zero(), FVector3f::Zero());
        // Edge extreme of the other edge
        FVector3f otherPoint;
        // Intersection with the other edge
        FVector3f secondIntersection;
        // List of edges that don't need an intersection to be tested (to avoid repeated intersection detection on edge extremes)
        TArray<ReflectorEdge> blackList = TArray<ReflectorEdge>();
        int e = 0;

        
        // For all planes of projection, check for intersections with the edges of this surface
        for (FVector3f normal : projectionPlanesNormals)
        {
            blackList.Empty();
            e = 0;

            while (e < beam.Edges().Num())
            {
                ReflectorEdge edge = beam.Edges()[e];

                // Checks if the edge intersects the plane
                if ( !blackList.Contains(edge) && LinePlaneIntersection( &intersection, edge.PointA, edge.PointB - edge.PointA, normal, _nodes[parent].Position ) )
                {
                    // Wether the intersection is on the extreme of the edge and the edge is entirely in the projection
                    bool doNothing = false;
                    // Wether the intersection is on the extreme of the edge and the edge is entirely out of the projection
                    bool intersectionOnExtreme = false;

                    // Check if intersection is on the edge extremes
                    if ((intersection - edge.PointA).Length() <= 0.02f)
                    {
                        if ( FVector3f::DotProduct( normal, (edge.PointB - _nodes[parent].Position).GetSafeNormal() ) >= 0 )
                        {
                            // The intersection is near the edge extreme A and the projection plane includes the other extreme, B
                            // The edge is included almost entirely, nothing to do here
                            doNothing = true;
                        }
                        else
                        {
                            // The intersection is near the edge extreme A and the projection plane excludes the other extreme, B
                            // The edge is excluded almost entirely, this information is saved
                            intersectionOnExtreme = true;
                        }
                    }
                    else if ((intersection - edge.PointB).Length() <= 0.02f)
                    {
                        if ( FVector3f::DotProduct( normal, (edge.PointA - _nodes[parent].Position).GetSafeNormal() ) >= 0 )
                        {
                            // The intersection is near the edge extreme B and the projection plane includes the other extreme, A
                            // The edge is included almost entirely, nothing to do here
                            doNothing = true;
                        }
                        else
                        {
                            // The intersection is near the edge extreme B and the projection plane excludes the other extreme, A
                            // The edge is excluded almost entirely, this information is saved
                            intersectionOnExtreme = true;
                        }
                    }


                    // If the edge is not entirely inside the projection
                    if (!doNothing)
                    {
                        // The point that is on the correct semispace of the plane (to be kept)
                        inPoint = FVector3f::DotProduct( normal, (edge.PointA - _nodes[parent].Position).GetSafeNormal() ) > FVector3f::DotProduct( normal, (edge.PointB - _nodes[parent].Position).GetSafeNormal() ) ? edge.PointA : edge.PointB;
                        // The point that is on the other semispace of the plane (to be removed)
                        outPoint = inPoint == edge.PointA ? edge.PointB : edge.PointA;

                        // Finds the other edge that the point to be removed belongs to
                        otherEdge = beam.FindOtherEdge(outPoint, inPoint);

                        // If the other edge has been found
                        if (!otherEdge.IsVoid())
                        {
                            // The other point to which outPoint is connected
                            otherPoint = otherEdge.PointA == outPoint ? otherEdge.PointB : otherEdge.PointA;

                            // Checks if the other edge has also an intersection with the same projection plane
                            if (LinePlaneIntersection( &secondIntersection, otherEdge.PointA, otherEdge.PointB - otherEdge.PointA, normal, _nodes[parent].Position))
                            {
                                // The second intersection is near the other point
                                if ((secondIntersection - otherPoint).Length() <= 0.02f)
                                {
                                    if (intersectionOnExtreme)
                                    {
                                        // Both this edge and the other are entirely out of the projection beam
                                        // Both are removed and a single edge connecting theit opposite extremes is created

                                        beam.RemovePoint(outPoint);

                                        beam.RemoveEdge(edge);
                                        beam.RemoveEdge(otherEdge);

                                        blackList.Add( beam.AddEdge(inPoint, otherPoint) );
                                    }
                                    else
                                    {
                                        // The other edge is entirely out of the projection beam, the current one isn't
                                        // Both are removed and two edges are created: inner point-intersection, intersection-other point

                                        beam.RemovePoint(outPoint);

                                        beam.RemoveEdge(edge);
                                        beam.RemoveEdge(otherEdge);

                                        beam.AddPoint(intersection);

                                        blackList.Add( beam.AddEdge(intersection, inPoint) );
                                        blackList.Add( beam.AddEdge(intersection, otherPoint) );
                                    }
                                }
                                // The second intersection is near the outpoint
                                else if ((intersection - outPoint).Length() <= 0.02f)
                                {
                                    blackList.Add(edge);
                                }
                                // The second intersection is not on the edge extremes
                                else
                                {
                                    if (intersectionOnExtreme)
                                    {
                                        beam.RemovePoint(outPoint);

                                        beam.RemoveEdge(edge);
                                        beam.RemoveEdge(otherEdge);

                                        beam.AddPoint(secondIntersection);

                                        blackList.Add( beam.AddEdge(inPoint, secondIntersection) );
                                        blackList.Add( beam.AddEdge(secondIntersection, otherPoint) );
                                    }
                                    else
                                    {
                                        beam.RemovePoint(outPoint);

                                        beam.RemoveEdge(edge);
                                        beam.RemoveEdge(otherEdge);

                                        beam.AddPoint(intersection);
                                        beam.AddPoint(secondIntersection);

                                        blackList.Add( beam.AddEdge(intersection, inPoint) );
                                        blackList.Add( beam.AddEdge(intersection, secondIntersection) );
                                        blackList.Add( beam.AddEdge(secondIntersection, otherPoint) );
                                    }
                                }
                            }
                            // No second intersection on the other edge
                            else
                            {
                                if (intersectionOnExtreme)
                                {
                                    beam.RemovePoint(outPoint);
                                    
                                    beam.RemoveEdge(edge);
                                    beam.RemoveEdge(otherEdge);

                                    beam.AddEdge(inPoint, otherPoint);
                                }
                                else
                                {
                                    beam.RemovePoint(outPoint);
                                    
                                    beam.RemoveEdge(edge);
                                    beam.RemoveEdge(otherEdge);

                                    beam.AddPoint(intersection);

                                    beam.AddEdge(intersection, inPoint);
                                    beam.AddEdge(intersection, otherPoint);
                                }
                            }

                            e = 0;

                        }
                        else
                        {
                            /*
                            blackList.Add(edge);
                            e++;
                            */

                            // If the other edge has not been found, some approximation error has occurred
                            // This means the projection is extremely small -> let's remove it altogether
                            _beam++;

                            if (_debugBeamTracing)
                            {
                                UE_LOG(LogTemp, Display, TEXT("Other edge not found for node %i"), i);
                                
                                _nodes.Add( IS(i, order, parent, surface, beam, false ) );
                                _nodes[i].Position = pos;
                                return true;
                            }
                            else
                            {
                                return false;
                            }
                        }
                    }
                    else
                    {
                        blackList.Add(edge);
                        e++;
                    }
                }
                else
                {
                    blackList.Add(edge);
                    e++;
                }
            }
        }

        // If the resulting projection consists of 2 or less points (it's just a line or a point), no IS created
        if (beam.Points().Num() <= 2 || beam.Edges().Num() <= 2)
        {
            _beam++;

            if (_debugBeamTracing)
            {
                UE_LOG(LogTemp, Display, TEXT("Only a line or point remains for node %i"), i);
                
                _nodes.Add( IS(i, order, parent, surface, beam, false ) );
                _nodes[i].Position = pos;
                return true;
            }
            else
            {
                return false;
            }
        }

        // Checking, for each point resulting from the projection, if it is in the correct semispace of all planes
        for (FVector3f point : beam.Points())
        {
            for (FVector3f normal : projectionPlanesNormals)
            {
                // If a point of the projection falls out of a semispace of the projection plane, then no IS is created
                if ( FVector3f::DotProduct( (point - _nodes[parent].Position).GetSafeNormal() , normal) < -0.5f )
                {
                    _beam++;

                    if (_debugBeamTracing)
                    {
                        /*
                        if (i == 42 || i == 44)
                        {
                            UE_LOG(LogTemp, Display, TEXT("A point falls outside the projection planes for node %i. Precisely, the yellow point falls outside the red plane"), i);
                            DrawDebugPoint(Rooms[0]->GetWorld(), FVector(point), 50, FColor::Yellow, false, 50000, 0);
                            DrawDebugSolidPlane(Rooms[0]->GetWorld(), FPlane(FVector(_nodes[parent].Position), FVector(normal)), FVector(_nodes[parent].Position), 50.0f, FColor::Red, false, 50000, 0);
                        }
                        */
                        
                        _nodes.Add( IS(i, order, parent, surface, beam, false ) );
                        _nodes[i].Position = pos;
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
        }
    }



    // IS is created and its position is given
    if (_beamClipping)
        _nodes.Add( IS(i, order, parent, surface, beam ) );
    else
        _nodes.Add( IS(i, order, parent, surface, ISBeamProjection( surface->Points() , surface->Edges() ) ) );

    _nodes[i].Position = pos;

    _realISs++;

    return true;
}



// Given an IS position and the portion of the surface on which it needs to be projected, returns the set of planes passing from the IS to each edge
TArray<FVector3f> ISTree::CreateProjectionPlanes(FVector3f position, ISBeamProjection BeamProjection)
{
    TArray<FVector3f> normals = TArray<FVector3f>();
    for (ReflectorEdge e : BeamProjection.Edges())
    {
        FVector3f normal = FVector3f::CrossProduct(e.PointA - position, e.PointB - position);

        normal = CheckNormal(normal, e.PointA, e.PointB, BeamProjection.Points()) ? normal : -normal;

        normals.Add(normal);
    }

    return normals;
}



// Given a vector, an edge and a set of points (forming a convex polygon), checks if said vector is pointing in the direction of all points except those on the edge
bool ISTree::CheckNormal(FVector3f normal, FVector3f pointA, FVector3f pointB, TArray<FVector3f> points)
{
    for(FVector3f point : points)
    {
        // Since the polygon forming the surface is convex, it's sufficient to check a single point that doesn't belong to the edge
        if (point != pointA && point != pointB)
            return FVector3f::DotProduct((point - pointA).GetSafeNormal(), normal) >= 0;
    }

    return true;
}



// Returns true if a plane and segment intersect, point of intersection is in output in the variable intersection
bool ISTree::LinePlaneIntersection(FVector3f* intersection, FVector3f linePoint, FVector3f lineVec, FVector3f planeNormal, FVector3f planePoint, double epsilon)
{
    *intersection = FVector3f::Zero();

    //calculate the distance between the linePoint and the line-plane intersection point
    float dotNumerator = FVector3f::DotProduct(planePoint - linePoint, planeNormal);
    float dotDenominator = FVector3f::DotProduct(lineVec.GetSafeNormal(), planeNormal);

    // Checks that plane and line are not parallel
    if ( FMath::Abs(dotDenominator) > epsilon)
    {
        float length = dotNumerator / dotDenominator;

        *intersection = linePoint + lineVec.GetSafeNormal() * length;

        if (length <= 0)
            *intersection = linePoint;
        else if (length >= lineVec.Length())
            *intersection = linePoint + lineVec;

        return length >= -0.02f && length <= lineVec.Length() + 0.02f;
    }
    else
    {
        // The line and plane are parallel (no intersection)
        return false;
    }
}



// For public access
TArray<IS*> ISTree::Nodes()
{
    TArray<IS*> nodes;

    for (int i = 0; i < _nodes.Num(); i++)
    {
        nodes.Add(&_nodes[i]);    
    }
    
	return nodes;
}



// All reflectors in the scene
TArray<AReflectorSurface*> ISTree::Surfaces()
{
    TArray<AReflectorSurface*> surfaces = TArray<AReflectorSurface*>();
    
    for (ARoom* room : Rooms)
    {
        if (room != nullptr)
        {
            surfaces.Append(room->Surfaces);
        }
    }

    return surfaces;
}



ISTree::~ISTree()
{
}
