#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReflectorSurface.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UEPLUGIN_ISREVERB_API UReflectorSurface : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UReflectorSurface();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	UPROPERTY(VisibleAnywhere)
	int ID;

		
};
