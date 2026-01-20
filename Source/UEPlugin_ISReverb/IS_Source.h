#pragma once

#include "CoreMinimal.h"
#include "ISTree.h"
#include "IS_Listener.h"
#include "IS_RoomTracker.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ObjectMacros.h"
#include "IS_Source.generated.h"



/**
 * Actor that tracks the position of a sound source in the scene.
 * Responsible for generating ISs and simulating sound rays for reverberation.
 */
UCLASS(Blueprintable)
class UEPLUGIN_ISREVERB_API AIS_Source : public AIS_RoomTracker
{
	GENERATED_BODY()

public:
    AIS_Source();

private:
    // Image Sources trees, one for each listener
    TMap<AIS_Listener*, ISTree> trees;

public:
    /* The room(s) the source is currently in. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FString Room;
    
    /* The trace channel for sound reflection. */
    UPROPERTY(EditAnywhere)
    TEnumAsByte<ECollisionChannel> TraceChannel;

    /* Set to true to enable using multithreading on CPU-heavy computations (recommended). */
    UPROPERTY(EditAnywhere)
    bool EnableMultithreading;

    /* Set to true to activate IS generation (only in play mode) */
    UPROPERTY(EditAnywhere)
    bool generateImageSources = false;

    /* Set to true to activate path generation and checking (only in play mode) */
    UPROPERTY(EditAnywhere)
    bool generateReflectionPaths = false;

    /* The maximum order of reflection to be computed */
    UPROPERTY(EditAnywhere)
    int order;

    /* Set to true to visualize ISs (performance heavy) */
    UPROPERTY(EditAnywhere)
    bool drawImageSources = false;

    //[Header("Optimizations")]

    /* Set true to remove all ISs that fall on the front side of their reflecting surface */
    UPROPERTY(EditAnywhere)
    bool WrongSideOfReflector = true;

    /* Set true to remove ISs if their parent's projection on its reflector doesn't fall on their reflector */
    UPROPERTY(EditAnywhere)
    bool BeamTracing = true;

    /* Set true to clip IS reflectors with their parent's projection upon them, for more accurate beam tracing */
    UPROPERTY(EditAnywhere)
    bool BeamClipping = true;

    //[Header("Visualize")]

    /* The minimum order of valid reflections to be visualized (included), set to -1 to disable */
    UPROPERTY(EditAnywhere)
    int MinOrder = -1;

    /* The maximum order of valid reflections to be visualized (included), set to -1 to disable */
    UPROPERTY(EditAnywhere)
    int MaxOrder = -1;

    //[Header("Debug")]

    /* Draws projection of beam points and beam edges upon the reflector plane */
    UPROPERTY(EditAnywhere)
    bool drawPlaneProjection = true;

    /* The id of the IS node to be visualized for debug, set to -1 to disable */
    UPROPERTY(EditAnywhere)
    int checkNode = -1;

    /* The id of this IS node's parent */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int parentNode = 0;

    /* Set true to create invalid nodes (removed by optimizations) in the list below, to check for accurate removal */
    UPROPERTY(EditAnywhere)
    bool debugBeamTracing;

    /* Nodes removed by optimization, list is always empty if the option above is set to false */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<int> inactiveNodes = TArray<int>();

    FCriticalSection treesLock;



protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Called upon changes made in the editor.
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;


    
private:
    // Generates Image Sources position with the given parameters
    UFUNCTION(BlueprintCallable)
    void GenerateISs();

    void GenerateISsLinear(AIS_Listener* listener, FVector3f position);

    void GenerateISsMT(AIS_Listener* listener, FVector3f position);

    TFuture<ISTree> CreateISTreeTask(AIS_Listener* listener, FVector3f position);
    
    // Generates paths for sound reflections, checking if the sound reaches the listener
    void GenerateAllReflectionPaths();

    void GenerateRP(AIS_Listener* listener);

    void GenerateRPLinear(AIS_Listener* listener);

    void GenerateRPMT(AIS_Listener* listener);

    // Draws and deletes helpers for all debug purposes, according to the properties
    UFUNCTION(BlueprintCallable)
    void DrawDebug();
    


public:
    // Returns 1 / -1 if a plane and line intersect, 0 if the plane and segment are parallel, point of intersection is in output in the variable intersection
    // Return 1 if intersection is obtained by adding to linePoint a positive multiple of lineVec,-1 otherwise
    static int LinePlaneIntersection(FVector3f* intersection, FVector3f linePoint, FVector3f lineVec, FVector3f planeNormal, FVector3f planePoint, double epsilon = 1e-6);



    // Returns true if two TArrays of room pointers have at least one room in common
    static bool RoomsInCommon(TArray<ARoom*> a, TArray<ARoom*> b);
    

protected:
    // Called every time OnEnter or OnExit add or remove a room
    UFUNCTION(BlueprintCallable)
    void UpdateCurrentRoom() override;

};
