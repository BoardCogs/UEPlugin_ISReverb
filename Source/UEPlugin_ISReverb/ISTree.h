#pragma once

#include "CoreMinimal.h"
#include "IS.h"
#include "ReflectorSurface.h"
#include "Room.h"



/**
 * A tree containing all ISs generated for a source using a room's reflectors. 
 */
class UEPLUGIN_ISREVERB_API ISTree
{
public:
	// CONSTRUCTOR
    // Creates a tree of Image Sources
    // n = number of surfaces
    // r = maximum order of reflections
    ISTree(int r, FVector3f sourcePos, TArray<ARoom*> rooms, bool wrongSideOfReflector, bool beamTracing, bool beamClipping, bool debugBeamTracing);

private:
	// PROPERTIES
	// Number of surfaces
    int _sn;
    
    // Maximum order of reflections
    int _ro;
    
    // All nodes of the tree, each one identifying an Image Source
    TArray<IS> _nodes = TArray<IS>();

    // Amount of ISs saved by not mirroring another IS on its reflector
    int _noDouble = 0;

    // Wheter to optimize by not generating ISs on the front side of a reflector
    bool _wrongSideOfReflector;

    // Amount of ISs saved by not generating ISs on the front side of a reflector
    int _wrongSide = 0;

    // Wheter to optimize by not generating ISs for surfaces that fall outside the beam of their parent IS
    bool _beamTracing;

    // Wheter to optimize by projecting the beam from an IS only using the portion of reflector that fell inside the beam of the parent IS
    bool _beamClipping;

    // Amount of ISs saved by using beam tracing and clipping
    int _beam = 0;

    // In case of debug, this counter stores the actual number of active ISs created
    int _realISs = 0;

    // Generates ISs that would be shaved by beam tracing and clipping as inactive ISs, allows to check wether the optimization is accurate or not
    bool _debugBeamTracing;

	TArray<AReflectorSurface*> _surfaces;

	// METHODS
    // All reflectors in the scene
    TArray<AReflectorSurface*> Surfaces();

    // This function checks all conditions for creating a new Image Source, then creates it if all are respected
    bool CreateIS(int i, int order, int parent, AReflectorSurface* surface, TArray<FVector3f> projectionPlanesNormals);

    // Given an IS position and the portion of the surface on which it needs to be projected, returns the set of planes passing from the IS to each edge
    TArray<FVector3f> CreateProjectionPlanes(FVector3f position, ISBeamProjection BeamProjection);

    // Given a vector, an edge and a set of points (forming a convex polygon), checks if said vector is pointing in the direction of all points
    bool CheckNormal(FVector3f normal, FVector3f pointA, FVector3f pointB, TArray<FVector3f> points);

public:
	// PROPERTIES
	// The Room from which to take surfaces
	TArray<ARoom*> Rooms;

	// METHODS
    // For public access
    TArray<IS*> Nodes();
    
    // Returns true if a plane and segment intersect, point of intersection is in output in the variable intersection
    static bool LinePlaneIntersection(FVector3f* intersection, FVector3f linePoint, FVector3f lineVec, FVector3f planeNormal, FVector3f planePoint, double epsilon = 1e-6);
	
	~ISTree();
};
