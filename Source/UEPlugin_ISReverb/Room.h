#pragma once

#include "ReflectorSurface.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Room.generated.h"

UCLASS(Blueprintable)
class UEPLUGIN_ISREVERB_API ARoom : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoom();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleDefaultsOnly, SkipSerialization)
	TArray<AReflectorSurface*> Surfaces;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int SurfaceNumber;

	UFUNCTION(BlueprintCallable)
	void GetReflectors();

};
