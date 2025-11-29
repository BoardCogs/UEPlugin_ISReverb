#pragma once

#include "CoreMinimal.h"
#include "IS_RoomTracker.h"
#include "IS_Listener.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class UEPLUGIN_ISREVERB_API AIS_Listener : public AIS_RoomTracker
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	bool ActiveListener;
	
};
