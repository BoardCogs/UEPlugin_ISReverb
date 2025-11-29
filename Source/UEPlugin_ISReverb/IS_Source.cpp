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



void AIS_Source::GenerateISPositions()
{
	bool _backUpDrawISs = drawImageSources;
	drawImageSources = false;

	TArray<AActor*> listeners; 
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AIS_Listener::StaticClass(), listeners);

	for (AActor* actor : listeners)
	{
		AIS_Listener* listener = Cast<AIS_Listener>(actor);

		if ( RoomsInCommon(listener->GetRooms(), _rooms) )
		{
			ISTree tree = ISTree(order, FVector3f( GetTransform().TransformPosition(FVector3d(0,0,0)) ), listener->GetRooms(), WrongSideOfReflector, BeamTracing, BeamClipping, debugBeamTracing);

			inactiveNodes.Empty();

			for (int i = 0 ; i < tree.Nodes().Num() ; i++)
			{
				if (!tree.Nodes()[i].Valid)
					inactiveNodes.Add(i);
			}	
		}
	}

	drawImageSources = _backUpDrawISs;
}



bool AIS_Source::RoomsInCommon(TArray<ARoom*> a, TArray<ARoom*> b)
{
	for (ARoom* room : a)
	{
		if (b.Contains(room))
			return true;
	}

	return false;
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
