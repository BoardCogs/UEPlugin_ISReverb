#pragma once

#include "IS_ReflectorSurface.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IS_Room.generated.h"



/**
 * A room, given bounds and surfaces that reflect sound.
 * Surfaces should approximate the room's shape and most prominent features.
 */
UCLASS(Blueprintable)
class UEPLUGIN_ISREVERB_API AIS_Room : public AActor
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

	// Called upon changes made in the editor.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

public:
	UPROPERTY(EditAnywhere)
	FString Name;
	
	// PROPERTIES
	UPROPERTY(EditAnywhere)
	bool VisibleInGame;
	
	UPROPERTY(VisibleDefaultsOnly, SkipSerialization)
	TArray<AIS_ReflectorSurface*> Surfaces;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int SurfaceNumber;

	// METHODS

	UFUNCTION(BlueprintCallable)
	void GetReflectors();

};
