#include "IS_ReflectorSurface.h"



// Sets default values
AIS_ReflectorSurface::AIS_ReflectorSurface()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}



// Called when the game starts or when spawned
void AIS_ReflectorSurface::BeginPlay()
{
	Super::BeginPlay();

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, GetName() );

	_points.Add( FVector3f( GetTransform().TransformPosition(FVector3d(50,50,0)) ) );
	_points.Add( FVector3f( GetTransform().TransformPosition(FVector3d(-50,50,0)) ) );
	_points.Add( FVector3f( GetTransform().TransformPosition(FVector3d(-50,-50,0)) ) );
	_points.Add( FVector3f( GetTransform().TransformPosition(FVector3d(50,-50,0)) ) );

	_edges.Add(IS_ReflectorEdge(_points[0], _points[1]));
	_edges.Add(IS_ReflectorEdge(_points[1], _points[2]));
	_edges.Add(IS_ReflectorEdge(_points[2], _points[3]));
	_edges.Add(IS_ReflectorEdge(_points[3], _points[0]));

	/* This code was moved in the Room class to be toggled for each room
	TArray<UStaticMeshComponent*> Planes;
	GetComponents<UStaticMeshComponent>(Planes);

	for (UStaticMeshComponent* plane : Planes)
	{
		if (plane->GetName() == "Plane")
		{
			plane->SetVisibility(false);	
		}
	}
	*/

	// Draw lines indicating surface edges and normals to check
	/*
	DrawDebugLine(GetWorld(), FVector(Origin()), FVector(Origin() + 50*Normal()), FColor::Blue, true, -1, 0, 1);

	for (ReflectorEdge Edge : _edges)
	{
		DrawDebugLine(GetWorld(), FVector(Edge.PointA), FVector(Edge.PointB), FColor::Blue, true, -1, 0, 1);
	}
	*/
}



// Returns the origin point of this surface in world coordinates
FVector3f AIS_ReflectorSurface::Origin()
{
	return FVector3f( GetTransform().TransformPosition(FVector3d(0,0,0)) );
}

// Returns the normal of this surface as a unit vector
FVector3f AIS_ReflectorSurface::Normal()
{
	return (FVector3f( GetTransform().TransformPosition(FVector3d(0,0,10)) ) - Origin()).GetSafeNormal();
}

// Returns all points of the surface polygon
TArray<FVector3f> AIS_ReflectorSurface::Points()
{
	return _points;
}

// Returns all edges of the surface polygon
TArray<IS_ReflectorEdge> AIS_ReflectorSurface::Edges()
{
	return _edges;
}

