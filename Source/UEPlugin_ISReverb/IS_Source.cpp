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



void AIS_Source::UpdateCurrentRoom()
{
	Super::UpdateCurrentRoom();

	Room = TEXT("Currently in");

	if (_rooms.Num() > 0)
	{
		Room.Append(TEXT(":"));
		
		for (ARoom* room : _rooms)
		{
			Room.Append(TEXT(" "));
			Room.Append(room->GetName());
		}

		Room.Append(TEXT("."));
	}
	else
	{
		Room.Append(TEXT(" no room."));
	}
}
