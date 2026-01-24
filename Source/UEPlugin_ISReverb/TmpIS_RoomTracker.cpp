#include "TmpIS_RoomTracker.h"

// Sets default values
ATmpIS_RoomTracker::ATmpIS_RoomTracker()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ATmpIS_RoomTracker::BeginPlay()
{
	Super::BeginPlay();
	
}



// Called whenever entering a room collider
void ATmpIS_RoomTracker::OnEnterRoomCollider(ATmpIS_Room* room)
{
	// If this room isn't already in its tracker, insert it
	if (!_rooms.Contains(room))
	{
		_counter.Add(room, 0);
		_rooms.Add(room);

		UpdateCurrentRoom();
	}

	// Increase the counter
	_counter.Add(room, _counter[room]++);
}



// Called whenever exiting a room collider
void ATmpIS_RoomTracker::OnExitRoomCollider(ATmpIS_Room* room)
{
	// Decrease the counter
	_counter.Add(room, _counter[room]--);

	if (_counter[room] > 0) return;

	// If the counter is at zero, the object has exited all colliders of this room, so it is removed from the tracker
	            
	_rooms.Remove(room);
	_counter.Remove(room);
	            
	UpdateCurrentRoom();
}



// Called every time OnEnter or OnExit add or remove a room
void ATmpIS_RoomTracker::UpdateCurrentRoom()
{
}



// Returns the rooms in which this object is present
TArray<ATmpIS_Room*> ATmpIS_RoomTracker::GetRooms()
{
	return _rooms;
}