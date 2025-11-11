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

	//DrawDebugLine();
}

