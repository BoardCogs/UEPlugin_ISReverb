#include "IS.h"

IS::IS(int i, int order, int parent, FVector3f pos, AIS_ReflectorSurface* surface, IS_BeamProjection beam)
{
	Index = i;
	Order = order;
	Parent = parent;
	Position = pos;
	Surface = surface;
	BeamPoints = beam;
	HasPath = true;
	Path = TArray<FVector3f>();
}



// Sets the path followed by the sound ray reflection for this IS, if present
void IS::SetPath(bool b, TArray<FVector3f> p)
{
	HasPath = b;
	Path = p;
}



IS::~IS()
{
}
