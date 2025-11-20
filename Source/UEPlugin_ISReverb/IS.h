#pragma once

#include "CoreMinimal.h"
#include "ISBeamProjection.h"

/**
 * An Image Source, representing a possible reflection path from sound source to listener along a specific set of surfaces.
 */
class UEPLUGIN_ISREVERB_API IS
{
public:
	// CONSTRUCTOR
	IS(int i, int o, int p, int s, ISBeamProjection beam, bool v = true);

	// PROPERTIES
	// The index of this Image Source in its ISTree
	int Index;

	// The order of this IS
	int Order;

	// The index of this Image Source's parent
	int Parent;

	// The index of this Image Source's reflector
	int Surface;

	// The position of the Image Source
	FVector3f Position = FVector3f::Zero();

	// The points and edges resulting from beam tracing on this surface's reflector from its parent IS
	ISBeamProjection BeamPoints = ISBeamProjection(TArray<FVector3f>(), TArray<ReflectorEdge>());

	// If false, the IS should have been removed
	bool Valid;

	// If true, the IS has a reflection path that reaches the listener
	bool HasPath = true;

	// The reflection path followed by sound to reach the listener
	TArray<FVector3f> Path;

	// METHODS
	// Sets the path followed by the sound ray as it bounces on reflectors, represented through each reflection point
	void SetPath(bool b, TArray<FVector3f> p);
	
	~IS();
};
