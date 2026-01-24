#pragma once

#include "CoreMinimal.h"
#include "IS_Room.h"
#include "GameFramework/Actor.h"
#include "IS_RoomTracker.generated.h"



/**
 * An actor that tracks the room(s) it's currently in.
 */
UCLASS(Abstract)
class UEPLUGIN_ISREVERB_API AIS_RoomTracker : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AIS_RoomTracker();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called whenever entering a room collider
	UFUNCTION(BlueprintCallable)
	void OnEnterRoomCollider(AIS_Room* room);
	
	// Called whenever exiting a room collider
	UFUNCTION(BlueprintCallable)
	void OnExitRoomCollider(AIS_Room* room);

	// Returns the rooms in which this object is present
	UFUNCTION(BlueprintCallable)
	TArray<AIS_Room*> GetRooms();
	
protected:
	TArray<AIS_Room*> _rooms;

	TMap<AIS_Room*, int> _counter;

	// Called every time OnEnter or OnExit add or remove a room
	virtual void UpdateCurrentRoom();

};
