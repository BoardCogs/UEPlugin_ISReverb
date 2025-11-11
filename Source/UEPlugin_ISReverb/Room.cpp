#include "Room.h"



// Sets default values
ARoom::ARoom()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}



// Called when the game starts or when spawned
void ARoom::BeginPlay()
{
	Super::BeginPlay();
	
	GetReflectors();
}



// Collects all reflector surfaces attached to this room
void ARoom::GetReflectors()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Getting reflectors"));
	
	TArray<UChildActorComponent*> ChildActors;
	GetComponents<UChildActorComponent>(ChildActors);

	for (UChildActorComponent* ChildActor : ChildActors)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, ChildActor->GetChildActorName().ToString() );
		if (ChildActor->GetChildActor()->IsA(AReflectorSurface::StaticClass()))
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Found one"));
			Surfaces.Add(Cast<AReflectorSurface>(ChildActor->GetChildActor()));
		}
	}

	int i = 0;
	for (AReflectorSurface* s : Surfaces)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Processing surface number %i"), i));
		s->ID = i;
		i++;
	}

	SurfaceNumber = i;
}

