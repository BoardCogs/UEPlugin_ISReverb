#include "IS_Tree.h"



IS_Tree::IS_Tree(int r, FVector3f sourcePos, TArray<AIS_Room*> rooms, bool parallelExecution, bool wrongSideOfReflector, bool preGenBackSideSurfaces, bool midGenBackSideSurfaces, bool beamTracing, bool beamClipping, float cutArea)
{
    if (r == 0)
        return;
    
    _ro = r;
    Rooms = rooms;
    _surfaces = Surfaces();
    _sn = _surfaces.Num();
    _wrongSideOfReflector = wrongSideOfReflector;
    _preBackSideSurfaces = preGenBackSideSurfaces;
    _midBackSideSurfaces = midGenBackSideSurfaces;
    _beamTracing = beamTracing;
    _beamClipping = beamClipping;
    _cutArea = cutArea;

    FDateTime StartTime = FDateTime::UtcNow();

    // Checking for surfaces that face completely away from each other
    if (_preBackSideSurfaces)
    {
        _backSideSurfacesList.Empty();
        CheckBackSideSurfaces();
    }

    TArray<int> firstNodeOfOrder = TArray{ 0, 0 };

    // Creating the first order ISs
    for (int i = 0; i < _sn ; i++)
    {
        // Computing the position of the new IS by mirroring its parent along the reflecting surface
        float d = FVector3f::DotProduct(_surfaces[i]->Normal(),sourcePos - _surfaces[i]->Origin());
        
        // Checking that the created IS would not be on the wrong side of the reflector, standing on the opposite side of the surface's normal
        if ( _wrongSideOfReflector && d <= 0 )
        {
            _wrongSide++;
            continue;
        }

        FVector3f pos = sourcePos - 2 * d * _surfaces[i]->Normal();

        _nodes.Add( IS( i, 1, -1, pos, _surfaces[i], IS_BeamProjection( _surfaces[i]->Points() , _surfaces[i]->Edges() ) ) );
        _realISs++;
    }

    FCriticalSection iLock;
    FCriticalSection nodesLock;
    FCriticalSection noDoubleLock;
    FCriticalSection wrongSideLock;
    FCriticalSection preBackSideLock;
    FCriticalSection midBackSideLock;
    FCriticalSection beamLock;
    FCriticalSection areaLock;
    FCriticalSection realISsLock;
    
    // Creating all ISs from second order onward
    for (int i = _nodes.Num(), order = 2 ; order <= _ro ; order++)
    {
        // Sets the first IS of the currently considered order of reflection
        firstNodeOfOrder.Add(i);

        if (parallelExecution)
        {
            ParallelFor(firstNodeOfOrder[order] - firstNodeOfOrder[order - 1],
            [&](int32 index)
            {
                int p = firstNodeOfOrder[order - 1] + index;
            
                nodesLock.Lock();
                FVector3f nodePos = _nodes[p].Position;
                IS_BeamProjection nodeBeam = _nodes[p].BeamPoints;
                AIS_ReflectorSurface* nodeSurface = _nodes[p].Surface;
                nodesLock.Unlock();
            
                // Beam projection planes for the parent are generated here, to avoid repeating the operation for each child
                TArray<FVector3f> projectionPlanesNormals = CreateProjectionPlanes( nodePos, nodeBeam );

                // Iterates on all surfaces, checking if a new IS can be derived from a reflection of the parent on them
                for (int s = 0 ; s < _sn ; s++)
                {
                    if ( CreateIS(order, p, _surfaces[s], projectionPlanesNormals, nodeSurface, nodePos, nodeBeam, nodesLock, noDoubleLock, wrongSideLock, preBackSideLock, midBackSideLock, beamLock, areaLock, realISsLock) )
                    {
                        iLock.Lock();
                        i++;
                        iLock.Unlock();
                    }
                }
            });
        }
        else
        {
            for (int index = 0 ; index < firstNodeOfOrder[order] - firstNodeOfOrder[order - 1] ; index++)
            {
                int p = firstNodeOfOrder[order - 1] + index;
                
                IS* node = &_nodes[p];
            
                // Beam projection planes for the parent are generated here, to avoid repeating the operation for each child
                TArray<FVector3f> projectionPlanesNormals = CreateProjectionPlanes( node->Position, node->BeamPoints );

                // Iterates on all surfaces, checking if a new IS can be derived from a reflection of the parent on them
                for (int s = 0 ; s < _sn ; s++)
                {
                    if ( CreateIS(order, p, _surfaces[s], projectionPlanesNormals, node->Surface, node->Position, node->BeamPoints, nodesLock, noDoubleLock, wrongSideLock, preBackSideLock, midBackSideLock, beamLock, areaLock, realISsLock) )
                        i++;
                }
            }
        }
    }

    int TimeElapsedInMs = (FDateTime::UtcNow() - StartTime).GetTotalMilliseconds();

    /*
    Sending to console a debug message showing the total number of ISs created and the amount saved by optimization.
    The number of not generated ISs is only the tip of the iceberg: their children would have also been generated.
    */
    UE_LOG(LogTemp, Display, TEXT("\nIS generation over in %i milliseconds\n"
                                  "Total number of ISs generated: %i\n"
                                  "Optimizations:\n"
                                  " - No reflection on same surface twice in a row: %i ISs removed\n"
                                  " - Wrong side of reflector: %i ISs removed\n"
                                  " - Pre-generation backside surfaces: %i ISs removed\n"
                                  " - Mid-generation backside surfaces: %i ISs removed\n"
                                  " - Beam tracing%hs: %i ISs removed\n"
                                  " - Cut area: %i ISs removed"),
                                  TimeElapsedInMs, _realISs, _noDouble, _wrongSide, _preBackSide, _midBackSide, (_beamClipping ? " + clipping" : ""), _beam, _area);
}



// This function checks all conditions for creating a new Image Source, then creates it if all are respected
bool IS_Tree::CreateIS(int order, int parent, AIS_ReflectorSurface* surface, TArray<FVector3f> projectionPlanesNormals, AIS_ReflectorSurface* parentSurface, FVector3f parentPos, IS_BeamProjection parentBeam, FCriticalSection& nodesLock, FCriticalSection& noDoubleLock, FCriticalSection& wrongSideLock, FCriticalSection& preBackSideLock, FCriticalSection& midBackSideLock, FCriticalSection& beamLock, FCriticalSection& areaLock, FCriticalSection& realISsLock)
{
	// 1
    // Checking that no IS is created identifying a reflection on the same surface twice in a row
    // This is because a double reflection is impossible assuming flat surfaces
    if ( surface == parentSurface )
    {
        noDoubleLock.Lock();
        _noDouble++;
        noDoubleLock.Unlock();
        
        return false;
    }


    
    // Checking that the created IS would not be on the wrong side of the reflector, standing on the opposite side of the surface's normal

    

    // 2
    // Computing the position of the new IS by mirroring its parent along the reflecting surface 
    float d = FVector3f::DotProduct( surface->Normal() , parentPos - surface->Origin() );
    
    if ( _wrongSideOfReflector && d <= 0 )
    {
        wrongSideLock.Lock();
        _wrongSide++;
        wrongSideLock.Unlock();
        
        return false;
    }



    // Computing the position of the new IS by mirroring its parent along the reflecting surface
    FVector3f pos = parentPos - 2 * d * surface->Normal();



    // 3.0
    // Checking that the two surfaces (in their entirety) are not one on the backside of the other
    if ( _preBackSideSurfaces )
    {
        // Checking if the two surfaces are listed together
        // Either the parent surface is in the list of this IS's surface
        if (_backSideSurfacesList.Contains(surface))
        {
            if (_backSideSurfacesList[surface].Contains(parentSurface))
            {
                preBackSideLock.Lock();
                _preBackSide++;
                preBackSideLock.Unlock();
                return false;
            }
        }

        // Or this IS's surface is in the list of the parent surface
        if (_backSideSurfacesList.Contains(parentSurface))
        {
            if (_backSideSurfacesList[parentSurface].Contains(surface))
            {
                preBackSideLock.Lock();
                _preBackSide++;
                preBackSideLock.Unlock();
                return false;
            }
        }
    }



    // 3.5
    // Checking that the two surfaces (in their entirety) are not one on the backside of the other
    if ( _midBackSideSurfaces )
    {
        FVector3f intersectionPoint;
        bool intersection = false;
        
        // Checking if the edges of the parent's beam projection are intersected by the plane of this IS's surface
        for (IS_ReflectorEdge edge : parentBeam.Edges())
        {
            if (LinePlaneIntersection( &intersectionPoint, edge.PointA, edge.PointB - edge.PointA, surface->Normal(), surface->Points()[0] ) )
            {
                intersection = true;
                break;
            }
        }

        // If the parent's beam projection is on the backside of the IS surface
        if ( !intersection && (parentBeam.Points()[0] - surface->Points()[0]).Dot(surface->Normal()) < 0 )
        {
            midBackSideLock.Lock();
            _midBackSide++;
            midBackSideLock.Unlock();

            return false;
        }
    }



    // 4
    // Checking that reflections from parent to this surface are possible with beam tracing (+ clipping)

    IS_BeamProjection beam = IS_BeamProjection( surface->Points() , surface->Edges() );

    if (_beamTracing)
    {
        // Intersection with this edge
        FVector3f intersection;
        // Edge extreme that falls inside projection
        FVector3f inPoint;
        // Edge extreme that falls outside projection
        FVector3f outPoint;
        // The other edge connected to the outPoint
        IS_ReflectorEdge otherEdge = IS_ReflectorEdge(FVector3f::Zero(), FVector3f::Zero());
        // Edge extreme of the other edge
        FVector3f otherPoint;
        // Intersection with the other edge
        FVector3f secondIntersection;
        // List of edges that don't need an intersection to be tested (to avoid repeated intersection detection on edge extremes)
        TArray<IS_ReflectorEdge> blackList = TArray<IS_ReflectorEdge>();
        int e = 0;

        
        // For all planes of projection, check for intersections with the edges of this surface
        for (FVector3f normal : projectionPlanesNormals)
        {
            blackList.Empty();
            e = 0;

            while (e < beam.Edges().Num())
            {
                IS_ReflectorEdge edge = beam.Edges()[e];

                if (blackList.Contains(edge))
                {
                    e++;
                    continue;
                }

                // Checks if the edge intersects the plane
                if (LinePlaneIntersection( &intersection, edge.PointA, edge.PointB - edge.PointA, normal, parentPos ) )
                {
                    // Wether the intersection is on the extreme of the edge and the edge is entirely in the projection
                    bool doNothing = false;
                    // Wether the intersection is on the extreme of the edge and the edge is entirely out of the projection
                    bool intersectionOnExtreme = false;

                    // Check if intersection is on the edge extremes
                    if ((intersection - edge.PointA).Length() <= 0.02f)
                    {
                        if ( FVector3f::DotProduct( normal, (edge.PointB - parentPos).GetSafeNormal() ) >= 0 )
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
                        if ( FVector3f::DotProduct( normal, (edge.PointA - parentPos).GetSafeNormal() ) >= 0 )
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
                        inPoint = FVector3f::DotProduct( normal, (edge.PointA - parentPos).GetSafeNormal() ) > FVector3f::DotProduct( normal, (edge.PointB - parentPos).GetSafeNormal() ) ? edge.PointA : edge.PointB;
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
                            if (LinePlaneIntersection( &secondIntersection, otherEdge.PointA, otherEdge.PointB - otherEdge.PointA, normal, parentPos))
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

                            return false;
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
            
            return false;
        }

        // Checking, for each point resulting from the projection, if it is in the correct semispace of all planes
        for (FVector3f point : beam.Points())
        {
            for (FVector3f normal : projectionPlanesNormals)
            {
                // If a point of the projection falls out of a semispace of the projection plane, then no IS is created
                if ( FVector3f::DotProduct( (point - parentPos).GetSafeNormal() , normal) < -1 )
                {
                    beamLock.Lock();
                    _beam++;
                    beamLock.Unlock();

                    return false;
                }
            }
        }
    }


    
    // 5
    // Projection area check
    if (_cutArea > 0)
    {
        if (ComputePolygonArea(&beam) < _cutArea)
        {
            areaLock.Lock();
            _area++;
            areaLock.Unlock();
            
            return false;   
        }
    }


    
    nodesLock.Lock();
    
    // IS is created and its position is given
    if (_beamClipping)
        _nodes.Add( IS(_nodes.Num(), order, parent, pos, surface, beam ) );
    else
        _nodes.Add( IS(_nodes.Num(), order, parent, pos, surface, IS_BeamProjection( surface->Points() , surface->Edges() ) ) );

    nodesLock.Unlock();

    realISsLock.Lock();
    _realISs++;
    realISsLock.Unlock();

    return true;
}



// This function checks, given two surfaces, whether one of them is completely behind the other
void IS_Tree::CheckBackSideSurfaces()
{
    FVector3f intersectionPoint;

    // Comparing all surfaces against each other
    for (int i = 0; i < _surfaces.Num(); i++)
    {
        for (int j = i + 1; j < _surfaces.Num(); j++)
        {
            bool intersection = false;
            bool isBehind = false;

            // Checking if the edges of surface i are intersected by the plane of surface j
            for (IS_ReflectorEdge edge : _surfaces[i]->Edges())
            {
                if (LinePlaneIntersection( &intersectionPoint, edge.PointA, edge.PointB - edge.PointA, _surfaces[j]->Normal(), _surfaces[j]->Points()[0] ) )
                {
                    intersection = true;
                    break;
                }
            }

            // If surface i is on the backside of surface j
            if ( !intersection && (_surfaces[i]->Points()[0] - _surfaces[j]->Points()[0]).Dot(_surfaces[j]->Normal()) < 0 )
            {
                isBehind = true;
            }
            else
            {
                intersection = false;

                // Checking if the edges of surface j are intersected by the plane of surface i
                for (IS_ReflectorEdge edge : _surfaces[j]->Edges())
                {
                    if (LinePlaneIntersection( &intersectionPoint, edge.PointA, edge.PointB - edge.PointA, _surfaces[i]->Normal(), _surfaces[i]->Points()[0] ) )
                    {
                        intersection = true;
                        break;
                    }
                }

                // If surface j is on the backside of surface i
                if ( !intersection && (_surfaces[j]->Points()[0] - _surfaces[i]->Points()[0]).Dot(_surfaces[i]->Normal()) < 0 )
                {
                    isBehind = true;
                }
            }

            // If one surface is completely on one side of the other, memorize the information to avoid generating IS between them
            if ( isBehind )
            {
                UE_LOG(LogTemp, Display, TEXT("Found backsided surfaces: %i and %i\n"), _surfaces[i]->ID, _surfaces[j]->ID);
                
                TArray<AIS_ReflectorSurface*> iBackSideSurfaces = TArray<AIS_ReflectorSurface*>();

                // Adding surface i and j to the list of surfaces that are on the backside of each other
                if (_backSideSurfacesList.Contains(_surfaces[i]))
                {
                    iBackSideSurfaces = _backSideSurfacesList[_surfaces[i]];
                    iBackSideSurfaces.Add(_surfaces[j]);
                    _backSideSurfacesList.Add( _surfaces[i], iBackSideSurfaces );
                }
                else
                {
                    iBackSideSurfaces.Add(_surfaces[j]);
                    _backSideSurfacesList.Add( _surfaces[i], iBackSideSurfaces );
                }
            }
            
        }
    }
}



// Given an IS position and the portion of the surface on which it needs to be projected, returns the set of planes passing from the IS to each edge
TArray<FVector3f> IS_Tree::CreateProjectionPlanes(FVector3f position, IS_BeamProjection BeamProjection)
{
    TArray<FVector3f> normals = TArray<FVector3f>();
    for (IS_ReflectorEdge e : BeamProjection.Edges())
    {
        FVector3f normal = FVector3f::CrossProduct(e.PointA - position, e.PointB - position);

        normal = CheckNormal(normal, e.PointA, e.PointB, BeamProjection.Points()) ? normal : -normal;

        normals.Add(normal);
    }

    return normals;
}



// Given a vector, an edge and a set of points (forming a convex polygon), checks if said vector is pointing in the direction of all points except those on the edge
bool IS_Tree::CheckNormal(FVector3f normal, FVector3f pointA, FVector3f pointB, TArray<FVector3f> points)
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
bool IS_Tree::LinePlaneIntersection(FVector3f* intersection, FVector3f linePoint, FVector3f lineVec, FVector3f planeNormal, FVector3f planePoint, double epsilon)
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



float IS_Tree::ComputePolygonArea(IS_BeamProjection* beam)
{
    float area = 0.0;

    FVector3f pointC = beam->Points()[0];
    
    for (IS_ReflectorEdge e : beam->Edges())
    {
        if (e.PointA != pointC && e.PointB != pointC)
        {
            area += FVector3f::CrossProduct(e.PointA - pointC, e.PointB - pointC).Length() / 2;
        }
    }
    
    return area;
}



// For public access
TArray<IS*> IS_Tree::Nodes()
{
    TArray<IS*> nodes;

    for (int i = 0; i < _nodes.Num(); i++)
    {
        nodes.Add(&_nodes[i]);
    }
    
	return nodes;
}



// All reflectors in the scene
TArray<AIS_ReflectorSurface*> IS_Tree::Surfaces()
{
    TArray<AIS_ReflectorSurface*> surfaces = TArray<AIS_ReflectorSurface*>();
    
    for (AIS_Room* room : Rooms)
    {
        if (room != nullptr)
        {
            surfaces.Append(room->Surfaces);
        }
    }

    return surfaces;
}



IS_Tree::~IS_Tree()
{
}
