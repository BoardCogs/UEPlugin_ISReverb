#pragma once

#include "ReflectorEdge.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReflectorSurface.generated.h"

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
	
private:
	TArray<FVector3d> Points;
	
	TArray<ReflectorEdge> Edges;
	
};
