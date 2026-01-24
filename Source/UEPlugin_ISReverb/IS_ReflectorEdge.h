#pragma once

#include "CoreMinimal.h"



/**
 * Edge of a reflector, defined by its two extremes expressed in world coordinates.
 */
class UEPLUGIN_ISREVERB_API IS_ReflectorEdge
{
public:
	// PROPERTIES
	FVector3f PointA;
	FVector3f PointB;

	// CONSTRUCTOR
	
	IS_ReflectorEdge(FVector3f a, FVector3f b);

	// METHODS

	FVector3f Direction();
	
	float Length();

	static IS_ReflectorEdge Void();

	bool IsVoid();
	
	~IS_ReflectorEdge();

protected:
	// METHODS
	friend bool operator==(const IS_ReflectorEdge& lhs, const IS_ReflectorEdge& rhs);
	
};
