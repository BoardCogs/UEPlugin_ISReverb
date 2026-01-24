#include "TmpIS.h"

TmpIS::TmpIS(int i, int order, int parent, FVector3f pos, ATmpIS_ReflectorSurface* surface, TmpIS_BeamProjection beam, bool valid)
{
	Index = i;
	Order = order;
	Parent = parent;
	Position = pos;
	Surface = surface;
	BeamPoints = beam;
	Valid = valid;
	HasPath = true;
	Path = TArray<FVector3f>();
}



// Sets the path followed by the sound ray reflection for this IS, if present
void TmpIS::SetPath(bool b, TArray<FVector3f> p)
{
	HasPath = b;
	Path = p;
}



TmpIS::~TmpIS()
{
}
