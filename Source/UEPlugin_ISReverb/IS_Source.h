#pragma once

#include <__msvc_ranges_to.hpp>

#include "CoreMinimal.h"
#include "ISTree.h"
#include "IS_RoomTracker.h"
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
	// The Image Sources tree
    ISTree tree = ISTree(0, 0, FVector3f::Zero(), nullptr, false, false, false, false);;

public:
    //[Tooltip("The layer mask for sound reflection")] [SerializeField]
    //LayerMask layerMask;

    //[Tooltip("The maximum order of reflection to be computed")] [SerializeField]
    UPROPERTY(EditAnywhere)
    int order;

    //[Tooltip("Set to true to activate IS generation (only in play mode)")] [SerializeField]
    UPROPERTY(EditAnywhere)
    bool generateImageSources = false;

    //[Tooltip("Set to true to activate path generation and checking (only in play mode)")] [SerializeField]
    UPROPERTY(EditAnywhere)
    bool generateReflectionPaths = false;

    //[Tooltip("Set to true to visualize ISs (performance heavy)")] [SerializeField]
    UPROPERTY(EditAnywhere)
    bool drawImageSources = false;

    //[Header("Optimizations")]

    //[Tooltip("Set true to remove all ISs that fall on the front side of their reflecting surface")] [SerializeField]
    UPROPERTY(EditAnywhere)
    bool WrongSideOfReflector = true;

    //[Tooltip("Set true to remove ISs if their parent's projection on its reflector doesn't fall on their reflector")] [SerializeField]
    UPROPERTY(EditAnywhere)
    bool BeamTracing = true;

    //[Tooltip("Set true to clip IS reflectors with their parent's projection upon them, for more accurate beam tracing")] [SerializeField]
    UPROPERTY(EditAnywhere)
    bool BeamClipping = true;

    //[Header("Visualize")]

    //[Tooltip("The minimum order of valid reflections to be visualized (included), set to -1 to disable")] [SerializeField]
    UPROPERTY(EditAnywhere)
    int MinOrder = -1;

    //[Tooltip("The maximum order of valid reflections to be visualized (included), set to -1 to disable")] [SerializeField]
    UPROPERTY(EditAnywhere)
    int MaxOrder = -1;

    //[Header("Debug")]

    //[Tooltip("Draws projection of beam points and beam edges upon the reflector plane")] [SerializeField]
    UPROPERTY(EditAnywhere)
    bool drawPlaneProjection = true;

    //[Tooltip("The id of the IS node to be visualized for debug, set to -1 to disable")] [SerializeField]
    UPROPERTY(EditAnywhere)
    int checkNode = -1;

    //[Tooltip("The id of this IS node's parent")] [SerializeField]
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int parentNode = 0;

    //[Tooltip("Set true to create invalid nodes (removed by optimization) in the list below, to check for accurate removal")] [SerializeField]
    UPROPERTY(EditAnywhere)
    bool debugBeamTracing;

    //[Tooltip("Nodes removed by optimization, list is always empty if the option above is set to false")] [SerializeField]
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



    /*
    // Activates node regeneration with the given parameters
    void OnValidate()
    {
        if (generateImageSources == true)
        {
            GenerateISPositions();
            generateImageSources = false;
        }

        if (generateReflectionPaths == true)
        {
            GenerateReflectionPaths();
            generateReflectionPaths = false;
        }
    }


    
    // Generates Image Sources position with the given parameters
    private void GenerateISPositions()
    {
        bool _backUpDrawISs = drawImageSources;
        drawImageSources = false;

        tree = new(SurfaceManager.Instance.N, order, transform.position, WrongSideOfReflector, BeamTracing, BeamClipping, debugBeamTracing);

        inactiveNodes.Clear();

        for (int i = 0 ; i < tree.Nodes.Count ; i++)
        {
            if (!tree.Nodes[i].valid)
                inactiveNodes.Add(i);
        }

        drawImageSources = _backUpDrawISs;
    }



    // Generates paths for sound reflections, checking if the sound reaches the listener
    private void GenerateReflectionPaths()
    {
        if (tree == null)
            return;

        float timePassed = Time.realtimeSinceStartup;

        int validPaths = 0;
        
        var listener = FindAnyObjectByType<AkAudioListener>().transform.position;

        RaycastHit hitInfo;
        List<Vector3> intersections = new();
        int currentIndex;
        IS currentNode;
        Vector3 from;
        Vector3 to;

        foreach (var node in tree.Nodes)
        {
            if (node.valid)
            {
                // Innocent until proven guilty
                node.hasPath = true;

                intersections.Clear();

                intersections.Add(listener);

                currentIndex = node.index;
                from = listener;

                while (currentIndex != -1)
                {
                    currentNode = tree.Nodes[currentIndex];
                    to = currentNode.position;

                    if ( Physics.Raycast(from, to - from, out hitInfo, Vector3.Magnitude(to - from), layerMask) )
                    {
                        var hitSurface = hitInfo.transform.gameObject.GetComponent<ReflectiveSurface>();

                        if ( hitSurface != null && hitSurface.id == Surfaces[currentNode.surface].id )
                        {
                            intersections.Add(hitInfo.point);
                            from = hitInfo.point;
                        }
                        else
                        {
                            intersections.Add(hitInfo.point);
                            node.hasPath = false;
                            break;
                        }
                    }
                    else
                    {
                        intersections.Add(to);
                        node.hasPath = false;
                        break;
                    }

                    currentIndex = currentNode.parent;
                }

                if (node.hasPath)
                {
                    if ( !Physics.Raycast(from, transform.position - from, out hitInfo, Vector3.Magnitude(transform.position - from), layerMask) )
                    {
                        intersections.Add(transform.position);
                        node.hasPath = true;
                        validPaths++;
                    }
                    else
                    {
                        intersections.Add(hitInfo.point);
                        node.hasPath = false;
                    }
                }

                node.path = new( intersections );
            }
        }

        timePassed = Time.realtimeSinceStartup - timePassed;

        Debug.Log(
                    "Reflection paths generated in " + timePassed * 1000 + " milliseconds \n" +
                    validPaths + " ISs with a valid path out of " + tree.Nodes.Count + " total ISs"
                 );
    }



    void OnDrawGizmos()
    {
        Gizmos.color = Color.red;

        // Draws Source
        Gizmos.DrawSphere(transform.position, 0.5f);

        Gizmos.color = Color.green;

        // Draws Image Sources
        if (drawImageSources == true)
        {
            if (tree != null)
            {
                foreach (var node in tree.Nodes)
                {
                    if (node.valid == true)
                        Gizmos.DrawSphere(node.position, 0.5f);
                }
            }
        }



        //Draw all reflections path in a given order interval
        if (MinOrder != -1 || MaxOrder != -1)
        {
            Gizmos.color = Color.black;

            foreach (var node in tree.Nodes)
            {
                if (node.order >= MinOrder && node.order <= MaxOrder && node.hasPath)
                {
                    for (int i = 0; i < node.path.Count - 1; i++)
                    {
                        Gizmos.DrawLine(node.path[i], node.path[i + 1]);
                    }
                }

                if (node.order > MaxOrder)
                    break;
            }
        }



        // Draws reflection path
        if (checkNode != -1 && checkNode >= 0 && checkNode < tree.Nodes.Count)
        {
            IS node = tree.Nodes[checkNode];

            if (node.hasPath == true)
                Gizmos.color = Color.black;
            else
                Gizmos.color = Color.red;

            for (int i = 0; i < node.path.Count - 1; i++)
            {
                Gizmos.DrawLine(node.path[i], node.path[i + 1]);
            }
        }


        
        // Draws beam tracing and clipping process to debug
        if (checkNode != -1 && checkNode >= SurfaceManager.Instance.N && checkNode < tree.Nodes.Count)
        {
            // Draws the resulting beam projection on the reflector

            Gizmos.color = Color.red;

            IS node = tree.Nodes[checkNode];

            // Displays the parent node index as a readonly field
            parentNode = node.parent;

            Gizmos.DrawSphere(node.position, 0.7f);

            foreach (var edge in node.beamPoints.Edges)
            {
                Gizmos.DrawLine(edge.pointA, edge.pointB);
            }

            // Creates plane on which the reflector lies

            Plane plane = new(node.beamPoints.Points[0], node.beamPoints.Points[1], node.beamPoints.Points[2]);

            // Draws the parent related gizmos

            Gizmos.color = Color.blue;

            IS nodeParent = tree.Nodes[node.parent];

            Gizmos.DrawSphere(nodeParent.position, 0.7f);

            // Draws parent beam points
            foreach (var edge in nodeParent.beamPoints.Edges)
            {
                Gizmos.DrawLine(edge.pointA, edge.pointB);
            }

            List<Vector3> intersections = new();
            List<int> checks = new();
            Vector3 intersection;

            // Saves projection intersections on reflector plane
            foreach (var point in nodeParent.beamPoints.Points)
            {
                int result = LinePlaneIntersection(out intersection, nodeParent.position, point-nodeParent.position, plane.normal, node.beamPoints.Points[0]);

                if (result == -1)
                {
                    intersections.Add(point + (point - nodeParent.position).normalized * 50);
                }
                else
                {
                    intersections.Add(intersection);
                }
                
                checks.Add(result);
            }

            // Draws projection beams
            foreach (var point in intersections)
            {
                Gizmos.DrawLine(nodeParent.position, point + (point - nodeParent.position).normalized * 50);
            }

            // Draws projection of beam points and beam edges upon the reflector plane
            if (drawPlaneProjection)
            {
            foreach (var edge in nodeParent.beamPoints.Edges)
                {
                    int indexA = nodeParent.beamPoints.Points.IndexOf(edge.pointA);
                    int indexB = nodeParent.beamPoints.Points.IndexOf(edge.pointB);

                    if (checks[indexA] == 1 && checks[indexB] == 1)
                    {
                        Gizmos.DrawLine(intersections[indexA], intersections[indexB]);
                    }
                }
            }
        }
    }



    // Returns true if a plane and segment intersect, point of intersection is in output in the variable intersection
    public static int LinePlaneIntersection(out Vector3 intersection, Vector3 linePoint, Vector3 lineVec, Vector3 planeNormal, Vector3 planePoint, double epsilon = 1e-6)
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

};
