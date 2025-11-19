#include "ISTree.h"

ISTree::ISTree(int n, int r, FVector3f sourcePos, ARoom room, bool wrongSideOfReflector, bool beamTracing, bool beamClipping, bool debugBeamTracing)
{
	if (r == 0)
        return;
    
    _sn = n;
    _ro = r;
    Room = &room;
    _wrongSideOfReflector = wrongSideOfReflector;
    _beamTracing = beamTracing;
    _beamClipping = beamClipping;
    _debugBeamTracing = debugBeamTracing;

    float timePassed = UGameplayStatics::GetTimeSeconds(Room->GetWorld());

    TArray<int> firstNodeOfOrder = TArray{ 0, 0 };

    TArray<FVector3f> projectionPlanesNormals = TArray<FVector3f>();

    // Creating the first order ISs
    for (int i = 0; i < _sn ; i++, _realISs++)
    {
        _nodes.Add( IS( i, 1, -1, i, ISBeamProjection( Surfaces()[i]->Points() , Surfaces()[i]->Edges() ) ) );

        FVector3f pos = sourcePos;
            
        pos -= 2 * Surfaces()[i]->Normal().Dot(pos - Surfaces()[i]->Origin()) * Surfaces()[i]->Normal();

        _nodes[i].Position = pos;

    }

    /*
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
                if ( CreateIS(i, order, p, s, projectionPlanesNormals) )
                    i++;
            }
        }
    }
    */

    timePassed = UGameplayStatics::GetTimeSeconds(Room->GetWorld()) - timePassed;

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



/*
// This function checks all conditions for creating a new Image Source, then creates it if all are respected
bool ISTree::CreateIS(int i, int order, int parent, int surface, TArray<FVector3f> projectionPlanesNormals)
{
	
}

// Given an IS position and the portion of the surface on which it needs to be projected, returns the set of planes passing from the IS to each edge
TArray<FVector3f> ISTree::CreateProjectionPlanes(FVector3f position, ISBeamProjection beam)
{
	
}

// Given a vector, an edge and a set of points (forming a convex polygon), checks if said vector is pointing in the direction of all points except those on the edge
bool ISTree::CheckNormal(FVector3f normal, FVector3f pointA, FVector3f pointB, TArray<FVector3f> points)
{
	
}

// For public access
TArray<IS> ISTree::Nodes()
{
	
}
    
// Returns true if a plane and segment intersect, point of intersection is in output in the variable intersection
bool ISTree::LinePlaneIntersection(FVector3f* intersection, FVector3f linePoint, FVector3f lineVec, FVector3f planeNormal, FVector3f planePoint, double epsilon = 1e-6)
{
	
}
*/



// All reflectors in the scene
TArray<AReflectorSurface*> ISTree::Surfaces()
{
    if (Room != nullptr)
    {
        return Room->Surfaces;
    }
    else
    {
        return TArray<AReflectorSurface*>();
    }
}



ISTree::~ISTree()
{
}
