#include "TmpIS_Tree.h"



TmpIS_Tree::TmpIS_Tree(int r, FVector3f sourcePos, TArray<ATmpIS_Room*> rooms, bool wrongSideOfReflector, bool beamTracing, bool beamClipping, bool debugBeamTracing)
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

    FDateTime StartTime = FDateTime::UtcNow();

    TArray<int> firstNodeOfOrder = TArray{ 0, 0 };

    // Creating the first order ISs
    for (int i = 0; i < _sn ; i++, _realISs++)
    {
        FVector3f pos = sourcePos;
        pos -= 2 * FVector3f::DotProduct(_surfaces[i]->Normal(),pos - _surfaces[i]->Origin()) * _surfaces[i]->Normal();

        _nodes.Add( TmpIS( i, 1, -1, pos, _surfaces[i], TmpIS_BeamProjection( _surfaces[i]->Points() , _surfaces[i]->Edges() ) ) );
    }

    FCriticalSection iLock;
    FCriticalSection nodesLock;
    FCriticalSection noDoubleLock;
    FCriticalSection wrongSideLock;
    FCriticalSection beamLock;
    FCriticalSection realISsLock;
    
    // Creating all ISs from second order onward
    for (int i = _sn, order = 2 ; order <= _ro ; order++)
    {
        // Sets the first IS of the currently considered order of reflection
        firstNodeOfOrder.Add(i);
        
        ParallelFor(firstNodeOfOrder[order] - firstNodeOfOrder[order - 1],
        [&](int32 index)
        {
            int p = firstNodeOfOrder[order - 1] + index;
            
            nodesLock.Lock();
            TmpIS* node = &_nodes[p];
            nodesLock.Unlock();
            
            if (node->Valid)
            {
                // Beam projection planes for the parent are generated here, to avoid repeating the operation for each child
                TArray<FVector3f> projectionPlanesNormals = CreateProjectionPlanes( node->Position, node->BeamPoints );

                // Iterates on all surfaces, checking if a new IS can be derived from a reflection of the parent on them
                for (int s = 0 ; s < _sn ; s++)
                {
                    if ( CreateIS(order, p, _surfaces[s], projectionPlanesNormals, nodesLock, noDoubleLock, wrongSideLock, beamLock, realISsLock) )
                    {
                        iLock.Lock();
                        i++;
                        iLock.Unlock();
                    }
                }   
            }
        });
    }

    int TimeElapsedInMs = (FDateTime::UtcNow() - StartTime).GetTotalMilliseconds();

    /*
    Sending to console a debug message showing the total number of ISs created and the amount saved by optimization.
    The number of not generated ISs is only the tip of the iceberg: their children would have also been generated.
    */
    UE_LOG(LogTemp, Display, TEXT("IS generation over in %i milliseconds\n"
                                  "Total number of ISs generated: %i\n"
                                  "Optimizations:\n"
                                  " - No reflection on same surface twice in a row: %i ISs removed\n"
                                  " - Wrong side of reflector: %i ISs removed\n"
                                  " - Beam tracing%hs: %i ISs removed"),
                                  TimeElapsedInMs, _realISs, _noDouble, _wrongSide, (_beamClipping ? " + clipping" : ""), _beam);
}



// This function checks all conditions for creating a new Image Source, then creates it if all are respected
bool TmpIS_Tree::CreateIS(int order, int parent, ATmpIS_ReflectorSurface* surface, TArray<FVector3f> projectionPlanesNormals, FCriticalSection& nodesLock, FCriticalSection& noDoubleLock, FCriticalSection& wrongSideLock, FCriticalSection& beamLock, FCriticalSection& realISsLock)
{
    nodesLock.Lock();
    TmpIS* parentNode = &_nodes[parent];
    nodesLock.Unlock();
    
	// 1
    // Checking that no IS is created identifying a reflection on the same surface twice in a row
    // This is because a double reflection is impossible assuming flat surfaces
    if ( surface == parentNode->Surface )
    {
        noDoubleLock.Lock();
        _noDouble++;
        noDoubleLock.Unlock();
        
        return false;
    }

    // Computing the position of the new IS by mirroring its parent along the reflecting surface
    FVector3f pos = parentNode->Position;
    pos -= 2 * FVector3f::DotProduct( surface->Normal() , pos - surface->Origin() ) * surface->Normal();



    // 2
    // Checking that the IS is not on the wrong side of the reflector, standing on the opposite side of the surface's normal
    if ( _wrongSideOfReflector && FVector3f::DotProduct( surface->Normal() , pos - surface->Origin() ) >= 0 )
    {
        wrongSideLock.Lock();
        _wrongSide++;
        wrongSideLock.Unlock();
        
        return false;
    }



    // 3
    // Checking that reflections from parent to this surface are possible with beam tracing (+ clipping)

    TmpIS_BeamProjection beam = TmpIS_BeamProjection( surface->Points() , surface->Edges() );

    if (_beamTracing)
    {
        // Intersection with this edge
        FVector3f intersection;
        // Edge extreme that falls inside projection
        FVector3f inPoint;
        // Edge extreme that falls outside projection
        FVector3f outPoint;
        // The other edge connected to the outPoint
        TmpIS_ReflectorEdge otherEdge = TmpIS_ReflectorEdge(FVector3f::Zero(), FVector3f::Zero());
        // Edge extreme of the other edge
        FVector3f otherPoint;
        // Intersection with the other edge
        FVector3f secondIntersection;
        // List of edges that don't need an intersection to be tested (to avoid repeated intersection detection on edge extremes)
        TArray<TmpIS_ReflectorEdge> blackList = TArray<TmpIS_ReflectorEdge>();
        int e = 0;

        
        // For all planes of projection, check for intersections with the edges of this surface
        for (FVector3f normal : projectionPlanesNormals)
        {
            blackList.Empty();
            e = 0;

            while (e < beam.Edges().Num())
            {
                TmpIS_ReflectorEdge edge = beam.Edges()[e];

                // Checks if the edge intersects the plane
                if ( !blackList.Contains(edge) && LinePlaneIntersection( &intersection, edge.PointA, edge.PointB - edge.PointA, normal, parentNode->Position ) )
                {
                    // Wether the intersection is on the extreme of the edge and the edge is entirely in the projection
                    bool doNothing = false;
                    // Wether the intersection is on the extreme of the edge and the edge is entirely out of the projection
                    bool intersectionOnExtreme = false;

                    // Check if intersection is on the edge extremes
                    if ((intersection - edge.PointA).Length() <= 0.02f)
                    {
                        if ( FVector3f::DotProduct( normal, (edge.PointB - parentNode->Position).GetSafeNormal() ) >= 0 )
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
                        if ( FVector3f::DotProduct( normal, (edge.PointA - parentNode->Position).GetSafeNormal() ) >= 0 )
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
                        inPoint = FVector3f::DotProduct( normal, (edge.PointA - parentNode->Position).GetSafeNormal() ) > FVector3f::DotProduct( normal, (edge.PointB - parentNode->Position).GetSafeNormal() ) ? edge.PointA : edge.PointB;
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
                            if (LinePlaneIntersection( &secondIntersection, otherEdge.PointA, otherEdge.PointB - otherEdge.PointA, normal, parentNode->Position))
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
                            beamLock.Lock();
                            _beam++;
                            beamLock.Unlock();

                            if (_debugBeamTracing)
                            {
                                nodesLock.Lock();
                                
                                _nodes.Add( TmpIS(_nodes.Num(), order, parent, pos, surface, beam, false ) );

                                nodesLock.Unlock();
                                
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
            beamLock.Lock();
            _beam++;
            beamLock.Unlock();

            if (_debugBeamTracing)
            {
                nodesLock.Lock();
                
                _nodes.Add( TmpIS(_nodes.Num(), order, parent, pos, surface, beam, false ) );

                nodesLock.Unlock();
                
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
                if ( FVector3f::DotProduct( (point - parentNode->Position).GetSafeNormal() , normal) < -1 )
                {
                    beamLock.Lock();
                    _beam++;
                    beamLock.Unlock();

                    if (_debugBeamTracing)
                    {
                        nodesLock.Lock();
                        
                        _nodes.Add( TmpIS(_nodes.Num(), order, parent, pos, surface, beam, false ) );

                        nodesLock.Unlock();
                        
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


    nodesLock.Lock();
    
    // IS is created and its position is given
    if (_beamClipping)
        _nodes.Add( TmpIS(_nodes.Num(), order, parent, pos, surface, beam ) );
    else
        _nodes.Add( TmpIS(_nodes.Num(), order, parent, pos, surface, TmpIS_BeamProjection( surface->Points() , surface->Edges() ) ) );

    nodesLock.Unlock();

    realISsLock.Lock();
    _realISs++;
    realISsLock.Unlock();

    return true;
}



// Given an IS position and the portion of the surface on which it needs to be projected, returns the set of planes passing from the IS to each edge
TArray<FVector3f> TmpIS_Tree::CreateProjectionPlanes(FVector3f position, TmpIS_BeamProjection BeamProjection)
{
    TArray<FVector3f> normals = TArray<FVector3f>();
    for (TmpIS_ReflectorEdge e : BeamProjection.Edges())
    {
        FVector3f normal = FVector3f::CrossProduct(e.PointA - position, e.PointB - position);

        normal = CheckNormal(normal, e.PointA, e.PointB, BeamProjection.Points()) ? normal : -normal;

        normals.Add(normal);
    }

    return normals;
}



// Given a vector, an edge and a set of points (forming a convex polygon), checks if said vector is pointing in the direction of all points except those on the edge
bool TmpIS_Tree::CheckNormal(FVector3f normal, FVector3f pointA, FVector3f pointB, TArray<FVector3f> points)
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
bool TmpIS_Tree::LinePlaneIntersection(FVector3f* intersection, FVector3f linePoint, FVector3f lineVec, FVector3f planeNormal, FVector3f planePoint, double epsilon)
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
TArray<TmpIS*> TmpIS_Tree::Nodes()
{
    TArray<TmpIS*> nodes;

    for (int i = 0; i < _nodes.Num(); i++)
    {
        nodes.Add(&_nodes[i]);
    }
    
	return nodes;
}



// All reflectors in the scene
TArray<ATmpIS_ReflectorSurface*> TmpIS_Tree::Surfaces()
{
    TArray<ATmpIS_ReflectorSurface*> surfaces = TArray<ATmpIS_ReflectorSurface*>();
    
    for (ATmpIS_Room* room : Rooms)
    {
        if (room != nullptr)
        {
            surfaces.Append(room->Surfaces);
        }
    }

    return surfaces;
}



TmpIS_Tree::~TmpIS_Tree()
{
}
