#pragma once

#include "Kismet/GameplayStatics.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IS_Room.generated.h"

class AIS_ReflectorSurface;



/**
 * A room, given bounds and surfaces that reflect sound.
 * Surfaces should approximate the room's shape and most prominent features.
 */
UCLASS(Blueprintable)
class ISREVERB_API AIS_Room : public AActor
{
	GENERATED_BODY()
	
public:
	// CONSTRUCTOR
	// Sets default values for this actor's properties
	AIS_Room();

protected:
	// METHODS
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Properties
	UPROPERTY(EditAnywhere)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool VisibleSurfaces = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<AIS_ReflectorSurface*> Surfaces;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int SurfaceNumber;

};
