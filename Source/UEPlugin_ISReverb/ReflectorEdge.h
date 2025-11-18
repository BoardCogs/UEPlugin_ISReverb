#pragma once

#include "CoreMinimal.h"

/**
 * Edge of a reflector, defined by its two extremes expressed in world coordinates.
 */
class UEPLUGIN_ISREVERB_API ReflectorEdge
{
public:
	FVector3f PointA;
	FVector3f PointB;
	
	ReflectorEdge(FVector3f a, FVector3f b);

	FVector3f Direction();
	
	float Length();

	bool IsVoid();
	
	~ReflectorEdge();

protected:

	friend bool operator==(const ReflectorEdge& lhs, const ReflectorEdge& rhs);
	
};
