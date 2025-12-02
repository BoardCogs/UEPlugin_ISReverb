#include "IS_Source.h"

// Sets default values
AIS_Source::AIS_Source()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}



void AIS_Source::BeginPlay()
{
	Super::BeginPlay();
	
}



void AIS_Source::GenerateISPositions()
{
	// Disable drawing ISs for faster computation
	bool _backUpDrawISs = drawImageSources;
	drawImageSources = false;

	// Getting all listeners
	TArray<AActor*> listeners; 
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AIS_Listener::StaticClass(), listeners);

	// Generating an ISTree for each listener in the level
	for (AActor* actor : listeners)
	{
		AIS_Listener* listener = Cast<AIS_Listener>(actor);

		// If listener and source are in the same room, generate ISs using that room's surfaces
		if ( RoomsInCommon(listener->GetRooms(), _rooms) )
		{
			// Generates ISTree and adds it to the array
			ISTree tree = ISTree(order, FVector3f( GetTransform().TransformPosition(FVector3d(0,0,0)) ), listener->GetRooms(), WrongSideOfReflector, BeamTracing, BeamClipping, debugBeamTracing);
			trees.Add(listener, tree);

			// If debug is active, the inactiveNodes Array is filled with indexes of ISs removed by optimizations, to check wether they work correctly
			if (debugBeamTracing)
			{
				inactiveNodes.Empty();

				for (int i = 0 ; i < tree.Nodes().Num() ; i++)
				{
					if (!tree.Nodes()[i].Valid)
						inactiveNodes.Add(i);
				}
			}
		}
	}

	drawImageSources = _backUpDrawISs;
}



void AIS_Source::GenerateReflectionPaths()
{
    if (trees.IsEmpty())
        return;

    float timePassed = UGameplayStatics::GetTimeSeconds(GetWorld());

    int validPaths = 0;
    
	for (TPair<AIS_Listener*, ISTree>& pair : trees)
	{
		FVector3f listener = FVector3f( pair.Key->GetTransform().TransformPosition(FVector3d(0,0,0)) );
		ISTree& tree = pair.Value;

		FHitResult hit;
		FCollisionQueryParams traceParams;
		
		TArray<FVector3f> intersections;
		
		int currentIndex;
		IS* currentNode;
		FVector3f from;
		FVector3f to;

		for (IS node : tree.Nodes())
		{
			if (node.Valid)
			{
				// Innocent until proven guilty
				node.HasPath = true;

				intersections.Empty();

				intersections.Add(listener);

				currentIndex = node.Index;
				from = listener;

				while (currentIndex != -1)
				{
					currentNode = &(tree.Nodes()[currentIndex]);
					to = currentNode->Position;

					if ( GetWorld()->LineTraceSingleByChannel(hit, FVector(from), FVector(to), TraceChannel, traceParams) )
					{
						AReflectorSurface* hitSurface = Cast<AReflectorSurface>( hit.GetActor() );

						if ( hitSurface != nullptr && hitSurface == currentNode->Surface )
						{
							intersections.Add( FVector3f( hit.ImpactPoint ) );
							from = FVector3f( hit.ImpactPoint );
						}
						else
						{
							intersections.Add( FVector3f( hit.ImpactPoint ) );
							node.HasPath = false;
							break;
						}
					}
					else
					{
						intersections.Add(to);
						node.HasPath = false;
						break;
					}

					currentIndex = currentNode->Parent;
				}

				if (node.HasPath)
				{
					if ( !GetWorld()->LineTraceSingleByChannel(hit, FVector(from), GetTransform().TransformPosition(FVector3d(0,0,0)), TraceChannel, traceParams) )
					{
						intersections.Add( FVector3f( GetTransform().TransformPosition(FVector3d(0,0,0)) ) );
						node.HasPath = true;
						validPaths++;
					}
					else
					{
						intersections.Add( FVector3f( hit.ImpactPoint ) );
						node.HasPath = false;
					}
				}

				node.Path = TArray(intersections);
			}
		}

		timePassed = UGameplayStatics::GetTimeSeconds(GetWorld()) - timePassed;

		UE_LOG(LogTemp, Display, TEXT("Reflection paths generated in %f milliseconds\n"
									  "%i ISs with a valid path out of %i total ISs"),
									  timePassed * 1000, validPaths, tree.Nodes().Num());
	}
}



/*
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
*/



bool AIS_Source::RoomsInCommon(TArray<ARoom*> a, TArray<ARoom*> b)
{
	for (ARoom* room : a)
	{
		if (b.Contains(room))
			return true;
	}

	return false;
}



void AIS_Source::UpdateCurrentRoom()
{
	Super::UpdateCurrentRoom();

	Room = TEXT("Currently in");

	if (_rooms.Num() > 0)
	{
		Room.Append(TEXT(":"));
		
		for (ARoom* room : _rooms)
		{
			Room.Append(TEXT(" "));
			Room.Append(room->GetName());
		}

		Room.Append(TEXT("."));
	}
	else
	{
		Room.Append(TEXT(" no room."));
	}
}
