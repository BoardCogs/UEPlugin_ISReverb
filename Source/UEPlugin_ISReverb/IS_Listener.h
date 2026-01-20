#pragma once

#include "CoreMinimal.h"
#include "IS_RoomTracker.h"
#include "IS_Listener.generated.h"



/**
 * Actor that tracks the position of a listener in the scene.
 */
UCLASS(Blueprintable)
class UEPLUGIN_ISREVERB_API AIS_Listener : public AIS_RoomTracker
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Room;
	
	bool ActiveListener;

protected:
	// Called every time OnEnter or OnExit add or remove a room
	UFUNCTION(BlueprintCallable)
	void UpdateCurrentRoom() override;
	
};
