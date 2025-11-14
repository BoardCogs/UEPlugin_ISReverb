#include "ReflectorSurface.h"



// Sets default values
AReflectorSurface::AReflectorSurface()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}



// Called when the game starts or when spawned
void AReflectorSurface::BeginPlay()
{
	Super::BeginPlay();
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Origin().ToString() );

	//DrawDebugLine();
}



FVector3f AReflectorSurface::Origin()
{
	return FVector3f( GetTransform().TransformPosition(FVector3d(0,0,0)) );
}

FVector3f AReflectorSurface::Normal()
{
	return FVector3f( GetTransform().TransformPosition(FVector3d(0,1,0)) ) - Origin();
}

TArray<FVector3f> AReflectorSurface::Points()
{
	return _points;
}

TArray<ReflectorEdge> AReflectorSurface::Edges()
{
	return _edges;
}

