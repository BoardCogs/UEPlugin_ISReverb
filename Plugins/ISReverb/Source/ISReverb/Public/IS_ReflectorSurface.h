#pragma once

#include "IS_ReflectorEdge.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IS_ReflectorSurface.generated.h"



/**
 * A surface upon which sound reflects.
 * A surface needs to be represented by a convex polygon, defined by its points and edges.
 */
UCLASS(Blueprintable)
class ISREVERB_API AIS_ReflectorSurface : public AActor
{
	GENERATED_BODY()
	
public:
	// CONSTRUCTOR
	// Sets default values for this actor's properties
	AIS_ReflectorSurface();

protected:
	// METHODS
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// PROPERTIES
	UPROPERTY(VisibleAnywhere)
	int ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Visible = false;

	/* 125 Hz absorption coefficient */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material properties", meta=(ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float Absorption125;

	/* 250 Hz absorption coefficient */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material properties", meta=(ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float Absorption250;

	/* 500 Hz absorption coefficient */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material properties", meta=(ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float Absorption500;

	/* 1000 Hz absorption coefficient */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material properties", meta=(ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float Absorption1000;

	/* 2000 Hz absorption coefficient */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material properties", meta=(ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float Absorption2000;

	/* 4000 Hz absorption coefficient */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material properties", meta=(ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float Absorption4000;
	

	// METHODS
	UFUNCTION(BlueprintCallable)
	FVector3f Origin();

	UFUNCTION(BlueprintCallable)
	FVector3f Normal();
	
	TArray<FVector3f> Points();
	
	TArray<IS_ReflectorEdge> Edges();
	
private:
	// PROPERTIES
	TArray<FVector3f> _points;
	
	TArray<IS_ReflectorEdge> _edges;
	
};
