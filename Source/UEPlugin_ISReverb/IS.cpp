#include "IS.h"

IS::IS(int i, int o, int p, int s, ISBeamProjection beam, bool v)
{
	Index = i;
	Order = o;
	Parent = p;
	Surface = s;
	BeamPoints = beam;
	Valid = v;
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
