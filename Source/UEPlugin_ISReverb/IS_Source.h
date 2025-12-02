#pragma once

#include <__msvc_ranges_to.hpp>

#include "CoreMinimal.h"
#include "ISTree.h"
#include "IS_Listener.h"
#include "IS_RoomTracker.h"
#include "UObject/ObjectMacros.h"
#include "IS_Source.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class UEPLUGIN_ISREVERB_API AIS_Source : public AIS_RoomTracker
{
	GENERATED_BODY()

public:
    AIS_Source();

private:
    // The Image Sources trees
    //ISTree tree = ISTree(0, 0, FVector3f::Zero(), nullptr, false, false, false, false);;

    // Image Sources trees, one for each listener
    TMap<AIS_Listener*, ISTree> trees;

public:
    /* The trace channel for sound reflection. */
    UPROPERTY(EditAnywhere)
    TEnumAsByte<ECollisionChannel> TraceChannel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FString Room;

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



protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;
    /*
    {
        GenerateISPositions();

        GenerateReflectionPaths();
    }
    */


    
private:
    // Generates Image Sources position with the given parameters
    UFUNCTION(BlueprintCallable)
    void GenerateISPositions();



    // Generates paths for sound reflections, checking if the sound reaches the listener
    UFUNCTION(BlueprintCallable)
    void GenerateReflectionPaths();
    


public:
    // Returns true if a plane and segment intersect, point of intersection is in output in the variable intersection
    static int LinePlaneIntersection(FVector3f* intersection, FVector3f linePoint, FVector3f lineVec, FVector3f planeNormal, FVector3f planePoint, double epsilon = 1e-6);
    /*
    {
        float length;
        float dotNumerator;
        float dotDenominator;
        intersection = Vector3.zero;

        //calculate the distance between the linePoint and the line-plane intersection point
        dotNumerator = Vector3.Dot(planePoint - linePoint, planeNormal);
        dotDenominator = Vector3.Dot(lineVec.normalized, planeNormal);

        // Checks that plane and line are not parallel
        if ( Math.Abs(dotDenominator) > epsilon)
        {
            length = dotNumerator / dotDenominator;

            intersection = linePoint + lineVec.normalized * length;

            return length > 0 ? 1 : -1;
        }
        else
        {
            // The line and plane are parallel (nothing to do)
            return 0;
        }
    }
    */



    // Returns true if two TArrays of room pointers have at least one room in common
    static bool RoomsInCommon(TArray<ARoom*> a, TArray<ARoom*> b);
    

protected:
    // Called every time OnEnter or OnExit add or remove a room
    UFUNCTION(BlueprintCallable)
    void UpdateCurrentRoom() override;

};
