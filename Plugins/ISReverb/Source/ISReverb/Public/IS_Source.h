#pragma once

#include "CoreMinimal.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "MetasoundSource.h"
#include "Components/AudioComponent.h"
#include "IS_Tree.h"
#include "IS_Listener.h"
#include "IS_RoomTracker.h"
#include "IS_SoundRayArray.h"
#include "RenderUtils.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ObjectMacros.h"
#include "IS_Source.generated.h"



/**
 * Actor that tracks the position of a sound source in the scene.
 * Responsible for generating ISs and simulating sound rays for reverberation.
 */
UCLASS(Blueprintable)
class ISREVERB_API AIS_Source : public AIS_RoomTracker
{
	GENERATED_BODY()

public:
    AIS_Source();

private:
    // All listeners in the scene (only one should be present)
    TArray<AIS_Listener*> _listeners;
    
    // Image Sources trees, one for each listener
    TMap<AIS_Listener*, IS_Tree> trees;

    // First buffer containing all simulated sound rays
    // If currentBuffer1 = true, it's the front buffer
    IS_SoundRayArray SoundRaysBuffer1 = IS_SoundRayArray();

    // Second buffer containing all simulated sound rays
    // If currentBuffer1 = false, it's the front buffer
    IS_SoundRayArray SoundRaysBuffer2 = IS_SoundRayArray();

    // Identifies front buffer and back buffer
    // The front buffer is the one with the most recent sound rays, switched to back buffer when the other is fully updated
    // The back buffer is the one filled when updating, before switching it to front buffer upon completion
    bool currentFrontBufferIs1 = false;

    // Array with all used Niagara effects
    TArray<UNiagaraComponent*> NiagaraEffects;

    // Timer since last sound played
    float timer;

    // Last source position for IS generation
    FVector3f LastSourcePos;

    // Last listener position for RP generation
    FVector3f LastListenerPos;

    // State of execution, true if either IS or RP generation is running
    bool currentlyExecuting = false;

    // Wether IS generation is cued to start as soon as the current operation finishes 
    bool cueISGeneration = false;

    // Wether RP generation is cued to start as soon as the current operation finishes
    bool cueRPGeneration = false;

public:
    /* The room(s) the source is currently in */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FString Room;
    
    /* The trace channel for sound reflection */
    UPROPERTY(EditAnywhere)
    TEnumAsByte<ECollisionChannel> TraceChannel;

    /* The Niagara System to spawn to visualize sound rays */
    UPROPERTY(EditAnywhere)
    UNiagaraSystem* SoundRayFX;

    /* The MateSound used to spawn audio */
    UPROPERTY(EditAnywhere)
    UMetaSoundSource* SoundEmitter;

    /* The audio attenuation settings */
    UPROPERTY(EditAnywhere)
    USoundAttenuation* SoundAttenuation;

    /* The audio to be played */
    UPROPERTY(EditAnywhere)
    USoundWave* SoundWave;

    /* Set to true to enable using multithreading on CPU-heavy computations (recommended) */
    UPROPERTY(EditAnywhere)
    bool EnableMultithreading;

    /* Set to true to activate IS generation (only in play mode) */
    //UPROPERTY(EditAnywhere)
    //bool generateImageSources = false;

    /* Set to true to activate path generation and checking (only in play mode) */
    //UPROPERTY(EditAnywhere)
    //bool generateReflectionPaths = false;

    /* Set to true to play sound from source (only in play mode) */
    //UPROPERTY(EditAnywhere)
    //bool playSound = false;

    /* The maximum order of reflection to be computed */
    UPROPERTY(EditAnywhere)
    int order;

    /* The distance after which ISs or ray paths are recomputed */
    UPROPERTY(EditAnywhere)
    int recomputeDistance = 150;

    /* Sound gain in dB */
    UPROPERTY(EditAnywhere)
    float soundGain = 0;

    //[Header("Optimizations")]

    /* Set true to remove all ISs that fall on the front side of their reflecting surface */
    UPROPERTY(EditAnywhere)
    bool WrongSideOfReflector = true;

    /* Set true to check for surfaces that face away from each other before IS generation and avoid testing them for ISs */
    UPROPERTY(EditAnywhere)
    bool BackSideSurfaces = true;

    /* Set true to remove ISs if their parent's projection on its reflector doesn't fall on their reflector */
    UPROPERTY(EditAnywhere)
    bool BeamTracing = true;

    /* Set true to clip IS reflectors with their parent's projection upon them, for more accurate beam tracing */
    UPROPERTY(EditAnywhere)
    bool BeamClipping = true;

    /* Projections with less are than this are discarded, set to <=0 to disable */
    UPROPERTY(EditAnywhere)
    float CutArea = 1000;

    //[Header("Visualize")]

    /* Maximum sound pressure level at 1 meter from source, in dB. Used only for graphical representation to show perception of sound */
    UPROPERTY(EditAnywhere)
    float soundLevel = 50;

    /* The minimum order of valid reflections to be visualized (included), set to -1 to disable */
    UPROPERTY(EditAnywhere)
    int MinOrder = -1;

    /* The maximum order of valid reflections to be visualized (included), set to -1 to disable */
    UPROPERTY(EditAnywhere)
    int MaxOrder = -1;

    /* Set to true to visualize ISs */
    UPROPERTY(EditAnywhere)
    bool drawImageSources = false;

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

    virtual void Tick(float DeltaSeconds) override;

    // Called upon changes made in the editor.
    //virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;


    
private:
    // Generates Image Sources position with the given parameters
    UFUNCTION(BlueprintCallable)
    void GenerateISs();

    void GenerateISsLinear(AIS_Listener* listener, FVector3f position);

    void GenerateISsMT(AIS_Listener* listener, FVector3f position);

    TFuture<IS_Tree> CreateISTreeTask(AIS_Listener* listener, FVector3f position);
    
    // Generates paths for sound reflections, checking if the sound reaches the listener
    UFUNCTION(BlueprintCallable)
    void GenerateAllReflectionPaths();

    void GenerateRP(AIS_Listener* listener);

    void GenerateRPLinear(AIS_Listener* listener);

    void GenerateRPMT(AIS_Listener* listener);

    // Draws and deletes helpers for all debug purposes, according to the properties
    UFUNCTION(BlueprintCallable)
    void DrawDebug();

    // Returns a pointer to the front sound ray buffer
    IS_SoundRayArray* GetFrontSoundRayBuffer();

    // Returns a pointer to the back sound ray buffer
    IS_SoundRayArray* GetBackSoundRayBuffer();
    


public:
    // Returns 1 / -1 if a plane and line intersect, 0 if the plane and segment are parallel, point of intersection is in output in the variable intersection
    // Return 1 if intersection is obtained by adding to linePoint a positive multiple of lineVec,-1 otherwise
    static int LinePlaneIntersection(FVector3f* intersection, FVector3f linePoint, FVector3f lineVec, FVector3f planeNormal, FVector3f planePoint, double epsilon = 1e-6);
    
    // Returns true if two TArrays of room pointers have at least one room in common
    static bool RoomsInCommon(TArray<AIS_Room*> a, TArray<AIS_Room*> b);
    
    // Plays assigned sound, applying reverberation according to the computed reflection paths
    UFUNCTION(BlueprintCallable)
    void PlaySound();
    

protected:
    // Called every time OnEnter or OnExit add or remove a room
    UFUNCTION(BlueprintCallable)
    void UpdateCurrentRoom() override;

};
