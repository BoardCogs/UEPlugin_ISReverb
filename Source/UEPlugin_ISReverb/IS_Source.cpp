#include "IS_Source.h"

// Sets default values
AIS_Source::AIS_Source()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}



void AIS_Source::BeginPlay()
{
	Super::BeginPlay();
	
}