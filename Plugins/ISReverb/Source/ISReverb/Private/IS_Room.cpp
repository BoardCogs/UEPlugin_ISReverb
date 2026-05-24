#include "IS_Room.h"



// Sets default values
AIS_Room::AIS_Room()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}



// Called when the game starts or when spawned
void AIS_Room::BeginPlay()
{
	Super::BeginPlay();
	
	// Giving all reflectors a room-relative id 
	int i = 0;
	for (AIS_ReflectorSurface* s : Surfaces)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Processing surface number %i"), i));
		s->ID = i;
		i++;
	}

	// Saving the number of surfaces of the room
	SurfaceNumber = i;
}