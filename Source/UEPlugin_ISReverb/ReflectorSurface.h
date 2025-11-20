#pragma once

#include "ReflectorEdge.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReflectorSurface.generated.h"

/**
 * A surface upon which sound reflects.
 * A surface needs to be represented by a convex polygon, defined by its points and edges.
 */
UCLASS(Blueprintable)
class UEPLUGIN_ISREVERB_API AReflectorSurface : public AActor
{
	GENERATED_BODY()
	
public:
	// CONSTRUCTOR
	// Sets default values for this actor's properties
	AReflectorSurface();

protected:
	// METHODS
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// PROPERTIES
	UPROPERTY(VisibleAnywhere)
	int ID;

	// METHODS
	UFUNCTION(BlueprintCallable)
	FVector3f Origin();

	UFUNCTION(BlueprintCallable)
	FVector3f Normal();
	
	TArray<FVector3f> Points();
	
	TArray<ReflectorEdge> Edges();
	
private:
	// PROPERTIES
	TArray<FVector3f> _points;
	
	TArray<ReflectorEdge> _edges;
	
};
