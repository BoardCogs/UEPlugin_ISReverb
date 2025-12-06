#include "IS_Listener.h"



void AIS_Listener::UpdateCurrentRoom()
{
	Super::UpdateCurrentRoom();

	Room = TEXT("Currently in");

	if (_rooms.Num() > 0)
	{
		Room.Append(TEXT(":"));
		
		for (ARoom* room : _rooms)
		{
			Room.Append(TEXT(" "));
			Room.Append(room->Name);
		}

		Room.Append(TEXT("."));
	}
	else
	{
		Room.Append(TEXT(" no room."));
	}
}