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

	GetComponents<UReflectorSurface>(Surfaces);

	int i = 0;
	for (UReflectorSurface* s : Surfaces)
	{
		s->ID = i;
		i++;
	}

	SurfaceNumber = i;
}

