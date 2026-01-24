#pragma once

#include "CoreMinimal.h"



/**
 * Edge of a reflector, defined by its two extremes expressed in world coordinates.
 */
class UEPLUGIN_ISREVERB_API TmpIS_ReflectorEdge
{
public:
	// PROPERTIES
	FVector3f PointA;
	FVector3f PointB;

	// CONSTRUCTOR
	
	TmpIS_ReflectorEdge(FVector3f a, FVector3f b);

	// METHODS

	FVector3f Direction();
	
	float Length();

	static TmpIS_ReflectorEdge Void();

	bool IsVoid();
	
	~TmpIS_ReflectorEdge();

protected:
	// METHODS
	friend bool operator==(const TmpIS_ReflectorEdge& lhs, const TmpIS_ReflectorEdge& rhs);
	
};
