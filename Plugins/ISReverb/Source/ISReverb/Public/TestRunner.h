#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RayGenTest.h"
#include "Engine/World.h"
#include "SceneInterface.h"
#include "../Private/ScenePrivate.h"
#include "TestRunner.generated.h"

UCLASS()
class ISREVERB_API ATestRunner : public AActor
{
	GENERATED_BODY()
	
public:	
	ATestRunner();
	FRayGenTest Test;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ShaderDemo)
	UTextureRenderTarget2D* RenderTarget = nullptr;
	
protected:
	virtual void BeginPlay() override;
	void UpdateTestParameters();

	float TranscurredTime; ///< allows us to add a delay on BeginPlay() 
	bool Initialized;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void BeginDestroy() override;
};