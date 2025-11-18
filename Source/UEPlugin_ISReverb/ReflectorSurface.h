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
	// Sets default values for this actor's properties
	AReflectorSurface();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UPROPERTY(VisibleAnywhere)
	int ID;

	UFUNCTION(BlueprintCallable)
	FVector3f Origin();

	UFUNCTION(BlueprintCallable)
	FVector3f Normal();
	
	TArray<FVector3f> Points();
	
	TArray<ReflectorEdge> Edges();
	
private:
	TArray<FVector3f> _points;
	
	TArray<ReflectorEdge> _edges;
	
};
