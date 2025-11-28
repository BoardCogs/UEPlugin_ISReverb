#pragma once

#include "CoreMinimal.h"
#include "Room.h"
#include "GameFramework/Actor.h"
#include "IS_RoomTracker.generated.h"

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
	void OnEnterRoomCollider(ARoom* room);
	
	// Called whenever exiting a room collider
	UFUNCTION(BlueprintCallable)
	void OnExitRoomCollider(ARoom* room);

	// Returns the rooms in which this object is present
	UFUNCTION(BlueprintCallable)
	TArray<ARoom*> GetRooms();
	
protected:
	TArray<ARoom*> _rooms;

	TMap<ARoom*, int> _counter;

	// Called every time OnEnter or OnExit are called to establish if the reverb needs to change
	void UpdateCurrentRoom();

};
