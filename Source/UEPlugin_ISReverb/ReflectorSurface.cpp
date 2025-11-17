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
	
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Origin().ToString() );

	_points.Add( FVector3f( GetTransform().TransformPosition(FVector3d(50,50,0)) ) );
	_points.Add( FVector3f( GetTransform().TransformPosition(FVector3d(-50,50,0)) ) );
	_points.Add( FVector3f( GetTransform().TransformPosition(FVector3d(-50,-50,0)) ) );
	_points.Add( FVector3f( GetTransform().TransformPosition(FVector3d(50,-50,0)) ) );

	_edges.Add(ReflectorEdge(_points[0], _points[1]));
	_edges.Add(ReflectorEdge(_points[1], _points[2]));
	_edges.Add(ReflectorEdge(_points[2], _points[3]));
	_edges.Add(ReflectorEdge(_points[3], _points[0]));

	DrawDebugLine(GetWorld(), FVector(Origin()), FVector(Origin() + Normal()), FColor::Blue, true, -1, 0, 1);

	for (ReflectorEdge Edge : _edges)
	{
		DrawDebugLine(GetWorld(), FVector(Edge.PointA), FVector(Edge.PointB), FColor::Blue, true, -1, 0, 1);
	}
}



FVector3f AReflectorSurface::Origin()
{
	return FVector3f( GetTransform().TransformPosition(FVector3d(0,0,0)) );
}

FVector3f AReflectorSurface::Normal()
{
	return /*(*/FVector3f( GetTransform().TransformPosition(FVector3d(0,0,10)) ) - Origin()/*).GetSafeNormal()*/;
}

TArray<FVector3f> AReflectorSurface::Points()
{
	return _points;
}

TArray<ReflectorEdge> AReflectorSurface::Edges()
{
	return _edges;
}

