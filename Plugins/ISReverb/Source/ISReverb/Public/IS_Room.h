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

	/* The room's name, which will be visualized by the sources and listener that are in them */
	UPROPERTY(EditAnywhere)
	FString Name;

	/* Sets the visibility of surfaces assigned to the room. Only works during execution */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool VisibleSurfaces = false;

	/* The list of surfaces assigned to this room and used to model it. Filled automatically during execution */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<AIS_ReflectorSurface*> Surfaces;

	/* The number of surfaces assigned to this room. Filled automatically during execution */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int SurfaceNumber;

};
