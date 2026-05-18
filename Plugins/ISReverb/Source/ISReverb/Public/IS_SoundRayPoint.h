#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "IS_SoundRayPoint.generated.h"

/**
 * A point in space where a sound ray hits a surface and is then reflected
 * The first point of a ray identifies the source, while the last is the listener
 */
UCLASS()
class ISREVERB_API UIS_SoundRayPoint : public UObject
{
	GENERATED_BODY()

public:
	UIS_SoundRayPoint();
	
	UIS_SoundRayPoint(FVector3f position, float inAmp, float outAmp);

	// The world position of this point
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector3f PointPosition;

	// Sound amplitude right before sound bounces off surface
	// This value is not present in the first point of each ray, as it indicates the source
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InAmplitude;
	
	// Sound amplitude right after sound bounces off surface
	// This value is not present in the last point of each ray, as it indicates the listener
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OutAmplitude;
	
};
