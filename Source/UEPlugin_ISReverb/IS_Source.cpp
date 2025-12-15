#include "IS_Source.h"

#include <string>

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



void AIS_Source::GenerateISs()
{
	// Getting all listeners
	TArray<AActor*> listeners; 
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AIS_Listener::StaticClass(), listeners);

	// Generating an ISTree for each listener in the level
	for (AActor* actor : listeners)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Inside listeners loop"));
		AIS_Listener* listener = Cast<AIS_Listener>(actor);

		FVector3f position;

		// If listener and source are in the same room, generate ISs using that room's surfaces
		if ( RoomsInCommon(listener->GetRooms(), _rooms) )
		{
			// Since the listener and source are in the same room, IS generation uses the source's actual position
			position = FVector3f( GetTransform().TransformPosition(FVector3d(0,0,0)) );
		}
		else
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Listener and source are in separate rooms"));

			// TODO: Set the source position after path finding
			position = FVector3f( GetTransform().TransformPosition(FVector3d(0,0,0)) );
		}

		// Generate tree
		if (!EnableMultithreading)
		{
			GenerateISsLinear(listener, position);
		}
		else
		{
			GenerateISsMT(listener, position);
		}
	}
}



void AIS_Source::GenerateISsLinear(AIS_Listener* listener, FVector3f position)
{
	// Generates ISTree and adds it to the array
	ISTree tree = ISTree(order, position, listener->GetRooms(), WrongSideOfReflector, BeamTracing, BeamClipping, debugBeamTracing);
	trees.Add(listener, tree);

	// If debug is active, the inactiveNodes Array is filled with indexes of ISs removed by optimizations, to check wether they work correctly
	if (debugBeamTracing)
	{
		inactiveNodes.Empty();
		TArray<IS*> nodes = tree.Nodes();

		for (int i = 0 ; i < nodes.Num() ; i++)
		{
			if (!nodes[i]->Valid)
				inactiveNodes.Add(i);
		}
	}

	GenerateRP(listener, trees[listener]);
}



void AIS_Source::GenerateISsMT(AIS_Listener* listener, FVector3f position)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Beginning async IS generation"));

	CreateISTreeTask(listener, position)
		.Next([this, listener](const ISTree& tree)
		{
			AsyncTask(ENamedThreads::GameThread, [this, listener, tree]()
			{
				trees.Add(listener, tree);
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Finished async IS generation"));

				GenerateRP(listener, trees[listener]);
			});
		});
}



TFuture<ISTree> AIS_Source::CreateISTreeTask(AIS_Listener* listener, FVector3f position)
{
	TSharedRef<TPromise<ISTree>> Promise = MakeShared<TPromise<ISTree>>();
	TFuture<ISTree> Future = Promise->GetFuture();

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, listener, position, Promise]() mutable
	{
		ISTree tree = ISTree(order, position, listener->GetRooms(), WrongSideOfReflector, BeamTracing, BeamClipping, debugBeamTracing);
		Promise->SetValue(tree);
	});

	return Future;
}



void AIS_Source::GenerateAllReflectionPaths()
{
    if (trees.IsEmpty())
        return;
    
	for (TPair<AIS_Listener*, ISTree>& pair : trees)
	{
		GenerateRP(pair.Key, pair.Value);
	}
}



void AIS_Source::GenerateRP(AIS_Listener* listener, ISTree& tree)
{
	GenerateRPLinear(listener, tree);

	/*
	if (EnableRayTracing)
	{
		//GenerateRPRT(listener, tree);
	}
	else if (EnableMultithreading)
	{
		//GenerateRPMT(listener, tree);
	}
	else
	{
		//GenerateRPLinear(listener, tree);
	}
	*/
}



void AIS_Source::GenerateRPLinear(AIS_Listener* listener, ISTree& tree)
{
	FDateTime StartTime = FDateTime::UtcNow();
	
	FVector3f listenerPos = FVector3f( listener->GetTransform().TransformPosition(FVector3d(0,0,0)) );

	FHitResult hit;
	FCollisionQueryParams traceParams;

	TArray<IS*> nodes = tree.Nodes();
	TArray<FVector3f> intersections;

	int validPaths = 0;
	int currentIndex;
	IS* currentNode = nullptr;
	FVector3f from;
	FVector3f to;

	for (IS* node : nodes)
	{
		if (node->Valid)
		{
			// Innocent until proven guilty
			node->HasPath = true;

			intersections.Empty();

			intersections.Add(listenerPos);

			currentIndex = node->Index;
			from = listenerPos;

			while (currentIndex != -1)
			{
				currentNode = nodes[currentIndex];
				to = currentNode->Position;

				//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Doing the line trace thing"));

				if ( GetWorld()->LineTraceSingleByChannel(hit, FVector(from + (to - from).GetSafeNormal() * 0.01f), FVector(to), TraceChannel, traceParams) )
				{
					AReflectorSurface* hitSurface = Cast<AReflectorSurface>( hit.GetActor() );

					//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Line trace thing hit something"));

					if ( hitSurface != nullptr && hitSurface == currentNode->Surface )
					{
						intersections.Add( FVector3f( hit.ImpactPoint ) );
						from = FVector3f( hit.ImpactPoint );
					}
					else
					{
						//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("But the wrong thing"));
						intersections.Add( FVector3f( hit.ImpactPoint ) );
						node->HasPath = false;
						break;
					}
				}
				else
				{
					intersections.Add(to);
					node->HasPath = false;
					break;
				}

				currentIndex = currentNode->Parent;
			}

			if (node->HasPath)
			{
				to = FVector3f( GetTransform().TransformPosition(FVector3d(0,0,0)) );
				
				if ( !GetWorld()->LineTraceSingleByChannel(hit, FVector(from + (to - from).GetSafeNormal() * 0.01f), FVector(to), TraceChannel, traceParams) )
				{
					intersections.Add( FVector3f( GetTransform().TransformPosition(FVector3d(0,0,0)) ) );
					node->HasPath = true;
					validPaths++;
				}
				else
				{
					intersections.Add( FVector3f( hit.ImpactPoint ) );
					node->HasPath = false;
				}
			}
			
			node->Path = TArray(intersections);
		}
	}

	int TimeElapsedInMs = (FDateTime::UtcNow() - StartTime).GetTotalMilliseconds();

	UE_LOG(LogTemp, Display, TEXT("Reflection paths generated in %i milliseconds\n"
								  "%i ISs with a valid path out of %i total ISs"),
								  TimeElapsedInMs, validPaths, nodes.Num());
}



void AIS_Source::GenerateRPMT(AIS_Listener* listener, ISTree& tree)
{
	
}



int AIS_Source::LinePlaneIntersection(FVector3f* intersection, FVector3f linePoint, FVector3f lineVec, FVector3f planeNormal, FVector3f planePoint, double epsilon)
{
	*intersection = FVector3f::Zero();

	// Calculate the distance between the linePoint and the line-plane intersection point
	float dotNumerator = FVector3f::DotProduct(planePoint - linePoint, planeNormal);
	float dotDenominator = FVector3f::DotProduct(lineVec.GetSafeNormal(), planeNormal);

	// Checks that plane and line are not parallel
	if ( FMath::Abs(dotDenominator) > epsilon)
	{
		float length = dotNumerator / dotDenominator;

		*intersection = linePoint + lineVec.GetSafeNormal() * length;

		return length > 0 ? 1 : -1;
	}
	else
	{
		// The line and plane are parallel (no intersection)
		return 0;
	}
}



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
			Room.Append(room->Name);
		}

		Room.Append(TEXT("."));
	}
	else
	{
		Room.Append(TEXT(" no room."));
	}
}



void AIS_Source::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// Saving a backup of current rooms
	TArray<ARoom*> RoomsBackup = _rooms;
	// Disabling all collisions (the engine will reset them upon calling Super anyway)
	SetActorEnableCollision(false);
	// Restoring current rooms
	_rooms = RoomsBackup;
	
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Getting the name of the changed variable
	FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;
	
	if (MemberPropertyName == "generateImageSources" || MemberPropertyName == "generateReflectionPaths")
	{
		if (GetWorld()->WorldType != EWorldType::Editor)
		{
			if (generateImageSources == true)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Generating ISs"));
				generateImageSources = false;
				GenerateISs();
			}
	
			if (generateReflectionPaths == true)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Generating reflections"));
				generateReflectionPaths = false;
				GenerateAllReflectionPaths();
			}

			DrawDebug();
		}
		else
		{
			// No IS generation and simulation unless the game is playing
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Not in editor mode please!"));
			generateImageSources = false;
			generateReflectionPaths = false;
		}
	}

	// If the property to draw image sources is toggled
	if (MemberPropertyName == "drawImageSources" || MemberPropertyName == "MinOrder" || MemberPropertyName == "MaxOrder" || MemberPropertyName == "drawPlaneProjection" || MemberPropertyName == "checkNode")
	{
		DrawDebug();
	}

	// Empty rooms
	_rooms.Empty();
	// Restore collisions (the engine will do its thing and all rooms will be back again)
	SetActorEnableCollision(true);

	/*
	 * P.S.: I know what I'm doing with the rooms seems out of place, but it's necessary.
	 * If you don't trust me, try removing the first three and last two lines of code and watch how nothing works
	 * when trying to call these functions from the edit page.
	*/
}



void AIS_Source::DrawDebug()
{
	FlushPersistentDebugLines(GetWorld());


	
	// Draws original source and ISs
	if (drawImageSources)
	{
		// Original source
		DrawDebugSphere(GetWorld(), GetTransform().TransformPosition(FVector3d(0,0,0)), 25, 12, FColor::Red, true, -1, 0, 2);

		// Image Sources
		if (trees.Num() > 0)
		{
			// Getting the first listener (tests should only be performed with one)
			TArray<AIS_Listener*> listeners; 
			trees.GetKeys(listeners);
    	
			for (IS* node : trees[listeners[0]].Nodes())
			{
				if (node->Valid == true)
					DrawDebugSphere(GetWorld(), FVector(node->Position), 25, 12, FColor::Green, true, -1, 0, 2);
			}
		}	
	}


	
	//Draw all reflections paths in a given order interval
	if (MinOrder != -1 || MaxOrder != -1)
	{
		if (trees.Num() > 0)
		{
			// Getting the first listener (tests should only be performed with one)
			TArray<AIS_Listener*> listeners; 
			trees.GetKeys(listeners);
    		
			for (IS* node : trees[listeners[0]].Nodes())
			{
				if (node->Order >= MinOrder && node->Order <= MaxOrder && node->HasPath)
				{
					for (int i = 0; i < node->Path.Num() - 1; i++)
					{
						DrawDebugLine(GetWorld(), FVector(node->Path[i]), FVector(node->Path[i + 1]), FColor::Black, true, -1, 0, 2);
					}
				}

				if (node->Order > MaxOrder)
					break;
			}
		}
	}
	

	
	// Draws reflection path for the node to check
	if (checkNode != -1)
	{
		if (trees.Num() > 0)
		{
			// Getting the first listener (tests should only be performed with one)
			TArray<AIS_Listener*> listeners; 
			trees.GetKeys(listeners);
			TArray<IS*> nodes = trees[listeners[0]].Nodes();
			
			if (checkNode >= 0 && checkNode < nodes.Num())
			{
				IS* node = nodes[checkNode];
				FColor color;

				if (node->HasPath == true)
					color = FColor::Black;
				else
					color = FColor::Red;

				for (int i = 0; i < node->Path.Num() - 1; i++)
				{
					DrawDebugLine(GetWorld(), FVector(node->Path[i]), FVector(node->Path[i + 1]), color, true, -1, 0, 2);
				}
			}
		}
	}


	
	// Draws beam tracing and clipping process for the node to check
	if (checkNode != -1)
	{
		if (trees.Num() > 0)
		{
			// Getting the first listener (tests should only be performed with one)
			TArray<AIS_Listener*> listeners; 
			trees.GetKeys(listeners);
			TArray<IS*> nodes = trees[listeners[0]].Nodes();
			
			if (checkNode >= 0 && checkNode < nodes.Num() && nodes[checkNode]->Parent != -1)
			{
				IS* node = nodes[checkNode];

				// Displays the parent node index as a readonly field
				parentNode = node->Parent;

				// Highlights the IS in red
				DrawDebugSphere(GetWorld(), FVector(node->Position), 30, 16, FColor::Red, true, -1, 0, 2);

				// Draws the resulting beam projection on the reflector
				for (ReflectorEdge edge : node->BeamPoints.Edges())
				{
					DrawDebugLine(GetWorld(), FVector(edge.PointA), FVector(edge.PointB), FColor::Red, true, -1, 0, 2);
				}

				// Creates normal of plane on which the reflector lies
				FVector3f planeNormal = FVector3f::CrossProduct(node->BeamPoints.Points()[1] - node->BeamPoints.Points()[0], node->BeamPoints.Points()[2] - node->BeamPoints.Points()[0]);

				// Draws the parent related gizmos
				//Gizmos.color = Color.blue;

				// Highlights the parent IS in blue
				IS* nodeParent = nodes[node->Parent];
				DrawDebugSphere(GetWorld(), FVector(nodeParent->Position), 30, 16, FColor::Blue, true, -1, 0, 2);

				// Draws parent beam points
				for (ReflectorEdge edge : nodeParent->BeamPoints.Edges())
				{
					DrawDebugLine(GetWorld(), FVector(edge.PointA), FVector(edge.PointB), FColor::Blue, true, -1, 0, 2);
				}

				TArray<FVector3f> intersections;
				TArray<int> checks;
				FVector3f intersection;

				// Saves projection intersections on reflector plane
				for (FVector3f point : nodeParent->BeamPoints.Points())
				{
					int result = LinePlaneIntersection(&intersection, nodeParent->Position, point - nodeParent->Position, planeNormal, node->BeamPoints.Points()[0]);

					if (result == -1)
					{
						intersections.Add(point + (point - nodeParent->Position).GetSafeNormal() * 1000);
					}
					else
					{
						intersections.Add(intersection);
					}
					
					checks.Add(result);
				}

				// Draws projection beams
				for (FVector3f point : intersections)
				{
					DrawDebugLine(GetWorld(), FVector(nodeParent->Position), FVector(point + (point - nodeParent->Position).GetSafeNormal() * 1000), FColor::Blue, true, -1, 0, 2);
				}

				// Draws projection of beam points and beam edges upon the reflector plane
				if (drawPlaneProjection)
				{
					for (ReflectorEdge edge : nodeParent->BeamPoints.Edges())
					{
						int indexA = nodeParent->BeamPoints.Points().IndexOfByKey(edge.PointA);
						int indexB = nodeParent->BeamPoints.Points().IndexOfByKey(edge.PointB);

						if (checks[indexA] == 1 && checks[indexB] == 1)
						{
							DrawDebugLine(GetWorld(), FVector(intersections[indexA]), FVector(intersections[indexB]), FColor::Blue, true, -1, 0, 2);
						}
					}
				}
			}
		}
	}
}