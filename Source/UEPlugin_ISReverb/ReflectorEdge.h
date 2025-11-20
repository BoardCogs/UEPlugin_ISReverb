#pragma once

#include "CoreMinimal.h"

/**
 * Edge of a reflector, defined by its two extremes expressed in world coordinates.
 */
class UEPLUGIN_ISREVERB_API ReflectorEdge
{
public:
	// PROPERTIES
	FVector3f PointA;
	FVector3f PointB;

	// CONSTRUCTOR
	
	ReflectorEdge(FVector3f a, FVector3f b);

	// METHODS

	FVector3f Direction();
	
	float Length();

	static ReflectorEdge Void();

	bool IsVoid();
	
	~ReflectorEdge();

protected:
	// METHODS
	friend bool operator==(const ReflectorEdge& lhs, const ReflectorEdge& rhs);
	
};
