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
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Getting reflectors"));
	
	TArray<UChildActorComponent*> ChildActors;
	GetComponents<UChildActorComponent>(ChildActors);

	// Checking all child actors
	for (UChildActorComponent* ChildActor : ChildActors)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, ChildActor->GetChildActorName().ToString() );

		// If a child actor is a reflector surface it goes into the array of surfaces
		if (ChildActor->GetChildActor()->IsA(AReflectorSurface::StaticClass()))
		{
			Surfaces.Add(Cast<AReflectorSurface>(ChildActor->GetChildActor()));

			// Turning surfaces invisible unless the VisibleInGame toggle is on 
			if (!VisibleInGame)
			{
				TArray<UStaticMeshComponent*> Planes;
				ChildActor->GetChildActor()->GetComponents<UStaticMeshComponent>(Planes);

				for (UStaticMeshComponent* plane : Planes)
				{
					if (plane->GetName() == "Plane")
					{
						plane->SetVisibility(false);	
					}
				}
			}
		}
	}

	// Giving all reflectors a room-relative id 
	int i = 0;
	for (AReflectorSurface* s : Surfaces)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Processing surface number %i"), i));
		s->ID = i;
		i++;
	}

	// Saving the number of surfaces of the room
	SurfaceNumber = i;
}



void ARoom::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	// If the changed property is VisibleInGame
	if (MemberPropertyName == "VisibleInGame")
	{
		// If the world is not currently in editor mode
		if (GetWorld()->WorldType != EWorldType::Editor)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("On it"));
			
			// Change visibility of all surfaces according to the property's value
			for (AReflectorSurface* s : Surfaces)
			{
				TArray<UStaticMeshComponent*> Planes;
				s->GetComponents<UStaticMeshComponent>(Planes);

				for (UStaticMeshComponent* plane : Planes)
				{
					if (plane->GetName() == "Plane")
					{
						plane->SetVisibility(VisibleInGame);
					}
				}
			}

			UpdateComponentVisibility();
		}	
	}
}
