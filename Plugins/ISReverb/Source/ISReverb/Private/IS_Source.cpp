#include "IS_Source.h"



// Sets default values
AIS_Source::AIS_Source()
{
	// Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}



void AIS_Source::BeginPlay()
{
	Super::BeginPlay();
}



void AIS_Source::Tick(float DeltaSeconds)
{
	// Recomputing
	if ( (FVector3f(this->GetTransform().GetLocation()) - LastSourcePos).Length() > recomputeDistance || cueISGeneration )
	{
		if (!currentlyExecuting)
		{
			cueISGeneration = false;
			cueRPGeneration = false;
			
			GenerateISs();
		}
		else
		{
			cueISGeneration = true;
		}
	}
	else
	{
		if (trees.IsEmpty()) {}
		else
		{
			for (TPair<AIS_Listener*, IS_Tree>& pair : trees)
			{
				if ( (FVector3f(pair.Key->GetTransform().GetLocation()) - LastListenerPos).Length() > recomputeDistance || cueRPGeneration )
				{
					if (!currentlyExecuting)
					{
						cueRPGeneration = false;
						
						GenerateRP(pair.Key);
					}
					else
					{
						cueRPGeneration = true;
					}
				}
			}
		}
	}

	// TODO: move play sound logic elsewhere

	// Playing sound regularly
	timer += DeltaSeconds;
	
	if (timer >= 2)
	{
		timer = 0;
		PlaySound();
	}
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
			position = FVector3f( GetTransform().TransformPosition(FVector3d(0,0,0)) );
		}

		LastSourcePos = FVector3f(this->GetTransform().GetLocation());

		// Generate tree
		if (EnableMultithreading)
		{
			GenerateISsMT(listener, position);
		}
		else
		{
			GenerateISsLinear(listener, position);
		}
	}
}



void AIS_Source::GenerateISsLinear(AIS_Listener* listener, FVector3f position)
{
	// Set state to currently executing
	currentlyExecuting = true;
	
	// Generates ISTree and adds it to the array
	IS_Tree tree = IS_Tree(order, position, listener->GetRooms(), WrongSideOfReflector, BeamTracing, BeamClipping, CutArea, debugBeamTracing);
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

	GenerateRP(listener);
}



void AIS_Source::GenerateISsMT(AIS_Listener* listener, FVector3f position)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Beginning async IS generation"));

	// Set state to currently executing
	currentlyExecuting = true;

	CreateISTreeTask(listener, position)
		.Next([this, listener](const IS_Tree& tree)
		{
			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, listener, tree]()
			{
				treesLock.Lock();
				trees.Add(listener, tree);
				treesLock.Unlock();
				
				//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Finished async IS generation"));

				GenerateRP(listener);
			});
		});
}



TFuture<IS_Tree> AIS_Source::CreateISTreeTask(AIS_Listener* listener, FVector3f position)
{
	TSharedRef<TPromise<IS_Tree>> Promise = MakeShared<TPromise<IS_Tree>>();
	TFuture<IS_Tree> Future = Promise->GetFuture();

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, listener, position, Promise]() mutable
	{
		IS_Tree tree = IS_Tree(order, position, listener->GetRooms(), WrongSideOfReflector, BeamTracing, BeamClipping, CutArea, debugBeamTracing);
		Promise->SetValue(tree);
	});

	return Future;
}



void AIS_Source::GenerateAllReflectionPaths()
{
    if (trees.IsEmpty())
        return;
    
	for (TPair<AIS_Listener*, IS_Tree>& pair : trees)
	{
		GenerateRP(pair.Key);
	}
}



void AIS_Source::GenerateRP(AIS_Listener* listener)
{
	LastListenerPos = FVector3f(listener->GetTransform().GetLocation());
	
	if (EnableMultithreading)
	{
		GenerateRPMT(listener);
	}
	else
	{
		GenerateRPLinear(listener);
	}
}



void AIS_Source::GenerateRPLinear(AIS_Listener* listener)
{
	FDateTime StartTime = FDateTime::UtcNow();
	
	FVector3f listenerPos = FVector3f( listener->GetTransform().TransformPosition(FVector3d(0,0,0)) );

	TArray<IS*> nodes = trees[listener].Nodes();

	IS_SoundRayArray* SoundRaysBackBuffer = GetBackSoundRayBuffer();
	SoundRaysBackBuffer->Empty();
	
	IS_SoundRay soundRay = IS_SoundRay();

	int validPaths = 0;

	for (IS* node : nodes)
	{
		FHitResult hit;
		FCollisionQueryParams traceParams;
		
		TArray<FVector3f> intersections;
		TArray<FVector3f> absorptions1;
		TArray<FVector3f> absorptions2;

		int currentIndex;
		IS* currentNode = nullptr;
		FVector3f from;
		FVector3f to;
		
		if (node->Valid)
		{
			// Innocent until proven guilty
			node->HasPath = true;

			intersections.Empty();
			absorptions1.Empty();
			absorptions2.Empty();

			intersections.Add(listenerPos);
			absorptions1.Add(FVector3f::Zero());
			absorptions2.Add(FVector3f::Zero());

			currentIndex = node->Index;
			from = listenerPos;

			// Iterating all checks going up the IS tree
			while (currentIndex != -1)
			{
				currentNode = nodes[currentIndex];
				to = currentNode->Position;

				//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Doing the line trace thing"));

				if ( GetWorld()->LineTraceSingleByChannel(hit, FVector(from + (to - from).GetSafeNormal() * 0.01f), FVector(to), TraceChannel, traceParams) )
				{
					AIS_ReflectorSurface* hitSurface = Cast<AIS_ReflectorSurface>( hit.GetActor() );

					//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Line trace thing hit something"));

					if ( hitSurface != nullptr && hitSurface == currentNode->Surface )
					{
						intersections.Add( FVector3f( hit.ImpactPoint ) );
						absorptions1.Add( FVector3f( hitSurface->Absorption125 , hitSurface->Absorption250 , hitSurface->Absorption500 ));
						absorptions2.Add( FVector3f( hitSurface->Absorption1000 , hitSurface->Absorption2000 , hitSurface->Absorption4000 ));
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

			// Final check from last intersection to source
			if (node->HasPath)
			{
				to = FVector3f( GetTransform().TransformPosition(FVector3d(0,0,0)) );
				
				if ( !GetWorld()->LineTraceSingleByChannel(hit, FVector(from + (to - from).GetSafeNormal() * 0.01f), FVector(to), TraceChannel, traceParams) )
				{
					intersections.Add( FVector3f( GetTransform().TransformPosition(FVector3d(0,0,0)) ) );
					absorptions1.Add(FVector3f::Zero());
					absorptions2.Add(FVector3f::Zero());
					node->HasPath = true;
					validPaths++;
				}
				else
				{
					intersections.Add( FVector3f( hit.ImpactPoint ) );
					node->HasPath = false;
				}
			}

			// Adding sound ray if valid
			if (node->HasPath)
			{
				soundRay.Clear();

				// Adding first point (source) and setting the position from which the sound arrives to listener
				soundRay.AddRayPoint( IS_SoundRayPoint( intersections[intersections.Num() - 1],
														-1, FMath::Max(soundLevel, 0.0),
														-1, FMath::Max(soundLevel, 0.0),
														-1, FMath::Max(soundLevel, 0.0),
														-1, FMath::Max(soundLevel, 0.0),
														-1, FMath::Max(soundLevel, 0.0),
														-1, FMath::Max(soundLevel, 0.0)
														  ) );
				soundRay.ISPosition = node->Position;

				// Sound amplitude is reduced by 6 dB each time distance doubles
				// In this case, the starting amplitude is assumed to be at 1 meter (100 units)
				float cumulativeDistance = 0.0f;
				// Total drop in amplitude due to cumulative distance from source
				float totalDrop;
				// Drop in amplitude in last ray point
				float lastAmplitudeDrop = 0.0f;
				// Drop in amplitude in this segment of the ray (total - last)
				float drop;
				// Amplitudes arriving at the ray point 
				float inAmp125;
				float inAmp250;
				float inAmp500;
				float inAmp1000;
				float inAmp2000;
				float inAmp4000;

				for (int i = intersections.Num() - 1; i > 1; i--)
				{
					// Adding the cumulative distance from source
					cumulativeDistance += (intersections[i] - intersections[i-1]).Length();
					
					// Total drop in amplitude is log2( totalDistance / initialDistance )
					// Where initialDistance is the distance of the original amplitude (again, assumed to be 1 meter)
					totalDrop = FMath::Log2( FMath::Max(cumulativeDistance, 100) / 100);
					
					// This is the factor by how much the amplitude drops in this segment of the ray
					drop = totalDrop - lastAmplitudeDrop;
					
					// Incoming amplitude is the outgoing amplitude of the last point minus the drop due to distance
					inAmp125 = soundRay.GetRayPoint(intersections.Num() - 1 - i)->OutAmplitude125 - (drop * 6);
					inAmp125 = FMath::Max(inAmp125, 0.0);
						
					inAmp250 = soundRay.GetRayPoint(intersections.Num() - 1 - i)->OutAmplitude250 - (drop * 6);
					inAmp250 = FMath::Max(inAmp250, 0.0);
						
					inAmp500 = soundRay.GetRayPoint(intersections.Num() - 1 - i)->OutAmplitude500 - (drop * 6);
					inAmp500 = FMath::Max(inAmp500, 0.0);

					inAmp1000 = soundRay.GetRayPoint(intersections.Num() - 1 - i)->OutAmplitude1000 - (drop * 6);
					inAmp1000 = FMath::Max(inAmp1000, 0.0);

					inAmp2000 = soundRay.GetRayPoint(intersections.Num() - 1 - i)->OutAmplitude2000 - (drop * 6);
					inAmp2000 = FMath::Max(inAmp2000, 0.0);

					inAmp4000 = soundRay.GetRayPoint(intersections.Num() - 1 - i)->OutAmplitude4000 - (drop * 6);
					inAmp4000 = FMath::Max(inAmp4000, 0.0);

					// Applying absorption to sound energy
					float outAmp125 = FMath::Pow(10.0, inAmp125 / 20 - 12.0) * (1.0 - absorptions1[i-1].X);
					outAmp125 = 20 * FMath::LogX(10, outAmp125 / FMath::Pow(10.0, -12.0));

					float outAmp250 = FMath::Pow(10.0, inAmp250 / 20 - 12.0) * (1.0 - absorptions1[i-1].Y);
					outAmp250 = 20 * FMath::LogX(10, outAmp250 / FMath::Pow(10.0, -12.0));

					float outAmp500 = FMath::Pow(10.0, inAmp500 / 20 - 12.0) * (1.0 - absorptions1[i-1].Z);
					outAmp500 = 20 * FMath::LogX(10, outAmp500 / FMath::Pow(10.0, -12.0));

					float outAmp1000 = FMath::Pow(10.0, inAmp1000 / 20 - 12.0) * (1.0 - absorptions2[i-1].X);
					outAmp1000 = 20 * FMath::LogX(10, outAmp1000 / FMath::Pow(10.0, -12.0));

					float outAmp2000 = FMath::Pow(10.0, inAmp2000 / 20 - 12.0) * (1.0 - absorptions2[i-1].Y);
					outAmp2000 = 20 * FMath::LogX(10, outAmp2000 / FMath::Pow(10.0, -12.0));

					float outAmp4000 = FMath::Pow(10.0, inAmp4000 / 20 - 12.0) * (1.0 - absorptions2[i-1].Z);
					outAmp4000 = 20 * FMath::LogX(10, outAmp4000 / FMath::Pow(10.0, -12.0));

					// Adding the ray point
					soundRay.AddRayPoint( IS_SoundRayPoint(intersections[i - 1],
														   inAmp125,outAmp125,
														   inAmp250,outAmp250,
														   inAmp500,outAmp500,
														   inAmp1000,outAmp1000,
														   inAmp2000,outAmp2000,
														   inAmp4000,outAmp4000)
														   );
					
					lastAmplitudeDrop = totalDrop;
				}

				// All operations are repeated for the last point
				cumulativeDistance += (intersections[0] - intersections[1]).Length();
				totalDrop = FMath::Log2( FMath::Max(cumulativeDistance, 100) / 100);
				drop = totalDrop - lastAmplitudeDrop;
				
				inAmp125 = soundRay.GetRayPoint(soundRay.GetNumRayPoints() - 1)->OutAmplitude125 - (drop * 6);
				inAmp125 = FMath::Max(inAmp125, 0.0);
					
				inAmp250 = soundRay.GetRayPoint(soundRay.GetNumRayPoints() - 1)->OutAmplitude250 - (drop * 6);
				inAmp250 = FMath::Max(inAmp250, 0.0);
					
				inAmp500 = soundRay.GetRayPoint(soundRay.GetNumRayPoints() - 1)->OutAmplitude500 - (drop * 6);
				inAmp500 = FMath::Max(inAmp500, 0.0);

				inAmp1000 = soundRay.GetRayPoint(soundRay.GetNumRayPoints() - 1)->OutAmplitude1000 - (drop * 6);
				inAmp1000 = FMath::Max(inAmp1000, 0.0);

				inAmp2000 = soundRay.GetRayPoint(soundRay.GetNumRayPoints() - 1)->OutAmplitude2000 - (drop * 6);
				inAmp2000 = FMath::Max(inAmp2000, 0.0);

				inAmp4000 = soundRay.GetRayPoint(soundRay.GetNumRayPoints() - 1)->OutAmplitude4000 - (drop * 6);
				inAmp4000 = FMath::Max(inAmp4000, 0.0);
					
				soundRay.AddRayPoint( IS_SoundRayPoint(intersections[0], inAmp125, -1, inAmp250, -1, inAmp500, -1, inAmp1000, -1, inAmp2000, -1, inAmp4000, -1) );
				soundRay.FinalAmplitudes1 = FVector3f(inAmp125,inAmp250,inAmp500);
				soundRay.FinalAmplitudes2 = FVector3f(inAmp1000,inAmp2000,inAmp4000);

				SoundRaysBackBuffer->AddRay(soundRay);
			}
			
			node->Path = TArray(intersections);
		}
	}

	int TimeElapsedInMs = (FDateTime::UtcNow() - StartTime).GetTotalMilliseconds();

	UE_LOG(LogTemp, Display, TEXT("Reflection paths generated in %i milliseconds\n"
								  "%i ISs with a valid path out of %i total ISs"),
								  TimeElapsedInMs, validPaths, nodes.Num());

	currentFrontBufferIs1 = !currentFrontBufferIs1;
								  
	DrawDebug();

	// Set state to not currently executing
	currentlyExecuting = false;
}



void AIS_Source::GenerateRPMT(AIS_Listener* listener)
{
	IS_SoundRayArray* SoundRaysBackBuffer = GetBackSoundRayBuffer();
	SoundRaysBackBuffer->Empty();
	
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, listener, SoundRaysBackBuffer]()
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Beginning async reflection paths generation"));
		
		FDateTime StartTime = FDateTime::UtcNow();
	
		FVector3f listenerPos = FVector3f( listener->GetTransform().TransformPosition(FVector3d(0,0,0)) );

		TArray<IS*> nodes = trees[listener].Nodes();

		int validISs = 0;

		FCriticalSection validISsLock;
		FCriticalSection SoundRaysLock;
		
		ParallelFor(nodes.Num(), [&](int32 index) mutable
		{
			IS* node = nodes[index];

			FHitResult hit;
			FCollisionQueryParams traceParams;
			
			TArray<FVector3f> intersections;
			TArray<FVector3f> absorptions1;
			TArray<FVector3f> absorptions2;

			int currentIndex;
			IS* currentNode = nullptr;
			FVector3f from;
			FVector3f to;

			if (node->Valid)
			{
				// Innocent until proven guilty
				node->HasPath = true;

				intersections.Empty();
				absorptions1.Empty();
				absorptions2.Empty();

				intersections.Add(listenerPos);
				absorptions1.Add(FVector3f::Zero());
				absorptions2.Add(FVector3f::Zero());

				currentIndex = node->Index;
				from = listenerPos;

				// Iterating all checks going up the IS tree
				while (currentIndex != -1)
				{
					currentNode = nodes[currentIndex];
					to = currentNode->Position;

					//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Doing the line trace thing"));

					if ( GetWorld()->LineTraceSingleByChannel(hit, FVector(from + (to - from).GetSafeNormal() * 0.01f), FVector(to), TraceChannel, traceParams) )
					{
						AIS_ReflectorSurface* hitSurface = Cast<AIS_ReflectorSurface>( hit.GetActor() );

						//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Line trace thing hit something"));

						if ( hitSurface != nullptr && hitSurface == currentNode->Surface )
						{
							intersections.Add( FVector3f( hit.ImpactPoint ) );
							absorptions1.Add( FVector3f( hitSurface->Absorption125 , hitSurface->Absorption250 , hitSurface->Absorption500 ));
							absorptions2.Add( FVector3f( hitSurface->Absorption1000 , hitSurface->Absorption2000 , hitSurface->Absorption4000 ));
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

				// Final check from last intersection to source
				if (node->HasPath)
				{
					to = FVector3f( GetTransform().TransformPosition(FVector3d(0,0,0)) );
					
					if ( !GetWorld()->LineTraceSingleByChannel(hit, FVector(from + (to - from).GetSafeNormal() * 0.01f), FVector(to), TraceChannel, traceParams) )
					{
						intersections.Add( FVector3f( GetTransform().TransformPosition(FVector3d(0,0,0)) ) );
						absorptions1.Add(FVector3f::Zero());
						absorptions2.Add(FVector3f::Zero());
						node->HasPath = true;

						validISsLock.Lock();
						validISs++;
						validISsLock.Unlock();
					}
					else
					{
						intersections.Add( FVector3f( hit.ImpactPoint ) );
						node->HasPath = false;
					}
				}

				// Adding sound ray if valid
				if (node->HasPath)
				{
					IS_SoundRay soundRay = IS_SoundRay();

					// Adding first point (source) and setting the position from which the sound arrives to listener
					soundRay.AddRayPoint( IS_SoundRayPoint( intersections[intersections.Num() - 1],
															-1, FMath::Max(soundLevel, 0.0),
															-1, FMath::Max(soundLevel, 0.0),
															-1, FMath::Max(soundLevel, 0.0),
															-1, FMath::Max(soundLevel, 0.0),
															-1, FMath::Max(soundLevel, 0.0),
															-1, FMath::Max(soundLevel, 0.0)
												          ) );
					soundRay.ISPosition = node->Position;

					// Sound amplitude is reduced by 6 dB each time distance doubles
					// In this case, the starting amplitude is assumed to be at 1 meter (100 units)
					float cumulativeDistance = 0.0f;
					// Total drop in amplitude due to cumulative distance from source
					float totalDrop;
					// Drop in amplitude in last ray point
					float lastAmplitudeDrop = 0.0f;
					// Drop in amplitude in this segment of the ray (total - last)
					float drop;
					// Amplitudes arriving at the ray point 
					float inAmp125;
					float inAmp250;
					float inAmp500;
					float inAmp1000;
					float inAmp2000;
					float inAmp4000;

					for (int i = intersections.Num() - 1; i > 1; i--)
					{
						// Adding the cumulative distance from source
						cumulativeDistance += (intersections[i] - intersections[i-1]).Length();
						
						// Total drop in amplitude is log2( totalDistance / initialDistance )
						// Where initialDistance is the distance of the original amplitude (again, assumed to be 1 meter)
						totalDrop = FMath::Log2( FMath::Max(cumulativeDistance, 100) / 100);
						
						// This is the factor by how much the amplitude drops in this segment of the ray
						drop = totalDrop - lastAmplitudeDrop;
						
						// Incoming amplitude is the outgoing amplitude of the last point minus the drop due to distance
						inAmp125 = soundRay.GetRayPoint(intersections.Num() - 1 - i)->OutAmplitude125 - (drop * 6);
						inAmp125 = FMath::Max(inAmp125, 0.0);
						
						inAmp250 = soundRay.GetRayPoint(intersections.Num() - 1 - i)->OutAmplitude250 - (drop * 6);
						inAmp250 = FMath::Max(inAmp250, 0.0);
						
						inAmp500 = soundRay.GetRayPoint(intersections.Num() - 1 - i)->OutAmplitude500 - (drop * 6);
						inAmp500 = FMath::Max(inAmp500, 0.0);

						inAmp1000 = soundRay.GetRayPoint(intersections.Num() - 1 - i)->OutAmplitude1000 - (drop * 6);
						inAmp1000 = FMath::Max(inAmp1000, 0.0);

						inAmp2000 = soundRay.GetRayPoint(intersections.Num() - 1 - i)->OutAmplitude2000 - (drop * 6);
						inAmp2000 = FMath::Max(inAmp2000, 0.0);

						inAmp4000 = soundRay.GetRayPoint(intersections.Num() - 1 - i)->OutAmplitude4000 - (drop * 6);
						inAmp4000 = FMath::Max(inAmp4000, 0.0);

						// Applying absorption to sound energy
						float outAmp125 = FMath::Pow(10.0, inAmp125 / 20 - 12.0) * (1.0 - absorptions1[i-1].X);
						outAmp125 = 20 * FMath::LogX(10, outAmp125 / FMath::Pow(10.0, -12.0));

						float outAmp250 = FMath::Pow(10.0, inAmp250 / 20 - 12.0) * (1.0 - absorptions1[i-1].Y);
						outAmp250 = 20 * FMath::LogX(10, outAmp250 / FMath::Pow(10.0, -12.0));

						float outAmp500 = FMath::Pow(10.0, inAmp500 / 20 - 12.0) * (1.0 - absorptions1[i-1].Z);
						outAmp500 = 20 * FMath::LogX(10, outAmp500 / FMath::Pow(10.0, -12.0));

						float outAmp1000 = FMath::Pow(10.0, inAmp1000 / 20 - 12.0) * (1.0 - absorptions2[i-1].X);
						outAmp1000 = 20 * FMath::LogX(10, outAmp1000 / FMath::Pow(10.0, -12.0));

						float outAmp2000 = FMath::Pow(10.0, inAmp2000 / 20 - 12.0) * (1.0 - absorptions2[i-1].Y);
						outAmp2000 = 20 * FMath::LogX(10, outAmp2000 / FMath::Pow(10.0, -12.0));

						float outAmp4000 = FMath::Pow(10.0, inAmp4000 / 20 - 12.0) * (1.0 - absorptions2[i-1].Z);
						outAmp4000 = 20 * FMath::LogX(10, outAmp4000 / FMath::Pow(10.0, -12.0));

						// Adding the ray point
						soundRay.AddRayPoint( IS_SoundRayPoint(intersections[i - 1],
											  inAmp125,outAmp125,
											  inAmp250,outAmp250,
											  inAmp500,outAmp500,
											  inAmp1000,outAmp1000,
											  inAmp2000,outAmp2000,
											  inAmp4000,outAmp4000)
									);
						
						lastAmplitudeDrop = totalDrop;
					}

					// All operations are repeated for the last point
					cumulativeDistance += (intersections[0] - intersections[1]).Length();
					totalDrop = FMath::Log2( FMath::Max(cumulativeDistance, 100) / 100);
					drop = totalDrop - lastAmplitudeDrop;
					
					inAmp125 = soundRay.GetRayPoint(soundRay.GetNumRayPoints() - 1)->OutAmplitude125 - (drop * 6);
					inAmp125 = FMath::Max(inAmp125, 0.0);
					
					inAmp250 = soundRay.GetRayPoint(soundRay.GetNumRayPoints() - 1)->OutAmplitude250 - (drop * 6);
					inAmp250 = FMath::Max(inAmp250, 0.0);
					
					inAmp500 = soundRay.GetRayPoint(soundRay.GetNumRayPoints() - 1)->OutAmplitude500 - (drop * 6);
					inAmp500 = FMath::Max(inAmp500, 0.0);

					inAmp1000 = soundRay.GetRayPoint(soundRay.GetNumRayPoints() - 1)->OutAmplitude1000 - (drop * 6);
					inAmp1000 = FMath::Max(inAmp1000, 0.0);

					inAmp2000 = soundRay.GetRayPoint(soundRay.GetNumRayPoints() - 1)->OutAmplitude2000 - (drop * 6);
					inAmp2000 = FMath::Max(inAmp2000, 0.0);

					inAmp4000 = soundRay.GetRayPoint(soundRay.GetNumRayPoints() - 1)->OutAmplitude4000 - (drop * 6);
					inAmp4000 = FMath::Max(inAmp4000, 0.0);
					
					soundRay.AddRayPoint( IS_SoundRayPoint(intersections[0], inAmp125, -1, inAmp250, -1, inAmp500, -1, inAmp1000, -1, inAmp2000, -1, inAmp4000, -1) );
					soundRay.FinalAmplitudes1 = FVector3f(inAmp125,inAmp250,inAmp500);
					soundRay.FinalAmplitudes2 = FVector3f(inAmp1000,inAmp2000,inAmp4000);

					SoundRaysLock.Lock();
					SoundRaysBackBuffer->AddRay(soundRay);
					SoundRaysLock.Unlock();
				}
				
				node->Path = TArray(intersections);
			}
		});

		int totalISs = nodes.Num();

		AsyncTask(ENamedThreads::GameThread, [this, StartTime, validISs, totalISs]()
		{
			int TimeElapsedInMs = (FDateTime::UtcNow() - StartTime).GetTotalMilliseconds();

			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Finished async reflection paths generation"));
			
			UE_LOG(LogTemp, Display, TEXT("Reflection paths generated in %i milliseconds\n"
										  "%i ISs with a valid path out of %i total ISs"),
										  TimeElapsedInMs, validISs, totalISs);

			currentFrontBufferIs1 = !currentFrontBufferIs1;
			
			DrawDebug();

			// Set state to not currently executing
			currentlyExecuting = false;
		});
	});
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



bool AIS_Source::RoomsInCommon(TArray<AIS_Room*> a, TArray<AIS_Room*> b)
{
	for (AIS_Room* room : a)
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
		
		for (AIS_Room* room : _rooms)
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
	TArray<AIS_Room*> RoomsBackup = _rooms;
	// Disabling all collisions (the engine will reset them upon calling Super anyway)
	SetActorEnableCollision(false);
	// Restoring current rooms
	_rooms = RoomsBackup;
	
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Getting the name of the changed variable
	FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;
	
	if (MemberPropertyName == "generateImageSources" || MemberPropertyName == "generateReflectionPaths" || MemberPropertyName == "playSound")
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

			if (playSound == true)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Playing sound"));
				playSound = false;
				PlaySound();
			}
		}
		else
		{
			// No IS generation and simulation unless the game is playing
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Not in editor mode please!"));
			generateImageSources = false;
			generateReflectionPaths = false;
			playSound = false;
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



void AIS_Source::PlaySound()
{
	if (SoundEmitter != nullptr)
	{
		// Getting the first listener (tests should only be performed with one)
		TArray<AIS_Listener*> listeners;
		trees.GetKeys(listeners);
    	FVector3f ListenerPosition = FVector3f(listeners[0]->GetTransform().GetLocation());

		// Spawning the original sound
		UAudioComponent* OriginalAudio = UGameplayStatics::SpawnSoundAttached(SoundEmitter, this->RootComponent, NAME_None, FVector::Zero(), FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true, 1, 1, 0, SoundAttenuation);
		
		if (OriginalAudio)
		{
			// Distance between source and listener
			float distance = (FVector3f(this->GetTransform().GetLocation()) - ListenerPosition).Length();
			// Distance in logarithmic scale
			float drop = FMath::Log2( FMath::Max(distance, 100) / 100);
			// Amplitude of the original sound is reduced by the drop due to distance from listener, roughly -6 dB each time distance doubles
			float amp = FMath::Max( soundLevel - (drop * 6) , 0.0 );
			// Amplitude normalized in [0,1] range
			float normalizedAmp = amp / soundLevel;

			OriginalAudio->SetWaveParameter(TEXT("Sound"), SoundWave);
			OriginalAudio->SetFloatParameter(TEXT("Delay"), distance / (343 * 100));
			OriginalAudio->SetFloatParameter(TEXT("Reflection125"), normalizedAmp);
			OriginalAudio->SetFloatParameter(TEXT("Reflection250"), normalizedAmp);
			OriginalAudio->SetFloatParameter(TEXT("Reflection500"), normalizedAmp);
			OriginalAudio->SetFloatParameter(TEXT("Reflection1000"), normalizedAmp);
			OriginalAudio->SetFloatParameter(TEXT("Reflection2000"), normalizedAmp);
			OriginalAudio->SetFloatParameter(TEXT("Reflection4000"), normalizedAmp);
		}

		// Getting the front sound ray buffer
		IS_SoundRayArray* SoundRaysFrontBuffer = GetFrontSoundRayBuffer();
		
		// Spawning reverb audio
		for (IS_SoundRay ray : SoundRaysFrontBuffer->SoundRays)
		{
			if (ray.FinalAmplitudes1.X / soundLevel < 0.01 && ray.FinalAmplitudes1.Y / soundLevel < 0.01 && ray.FinalAmplitudes1.Z / soundLevel < 0.01 && ray.FinalAmplitudes2.X / soundLevel < 0.01 && ray.FinalAmplitudes2.Y / soundLevel < 0.01 && ray.FinalAmplitudes2.Z / soundLevel < 0.01)
			{
				continue;
			}
			
			UAudioComponent* ReverbAudio = UGameplayStatics::SpawnSoundAtLocation(this, SoundEmitter, FVector(ray.ISPosition), FRotator::ZeroRotator, 1, 1, 0, SoundAttenuation );
		
			if (ReverbAudio)
			{
				ReverbAudio->SetWaveParameter(TEXT("Sound"), SoundWave);
				ReverbAudio->SetFloatParameter(TEXT("Delay"), (ray.ISPosition - ListenerPosition).Length() / (343 * 100));
				ReverbAudio->SetFloatParameter(TEXT("Reflection125"), ray.FinalAmplitudes1.X / soundLevel);
				ReverbAudio->SetFloatParameter(TEXT("Reflection250"), ray.FinalAmplitudes1.Y / soundLevel);
				ReverbAudio->SetFloatParameter(TEXT("Reflection500"), ray.FinalAmplitudes1.Z / soundLevel);
				ReverbAudio->SetFloatParameter(TEXT("Reflection1000"), ray.FinalAmplitudes2.X / soundLevel);
				ReverbAudio->SetFloatParameter(TEXT("Reflection2000"), ray.FinalAmplitudes2.Y / soundLevel);
				ReverbAudio->SetFloatParameter(TEXT("Reflection4000"), ray.FinalAmplitudes2.Z / soundLevel);
			}
		}
	}
}



void AIS_Source::DrawDebug()
{
	FlushPersistentDebugLines(GetWorld());


	
	// Draws original source and ISs
	if (drawImageSources)
	{
		// Original source
		DrawDebugPoint(GetWorld(), GetTransform().TransformPosition(FVector3d(0,0,0)), 10, FColor::Red, true, -1);
		//DrawDebugSphere(GetWorld(), GetTransform().TransformPosition(FVector3d(0,0,0)), 25, 12, FColor::Red, true, -1, 0, 2);

		// Image Sources
		if (trees.Num() > 0)
		{
			// Getting the first listener (tests should only be performed with one)
			TArray<AIS_Listener*> listeners; 
			trees.GetKeys(listeners);
    	
			for (IS* node : trees[listeners[0]].Nodes())
			{
				if (node->Valid == true)
					DrawDebugPoint(GetWorld(), FVector(node->Position), 10, FColor::Green, true, -1);
					//DrawDebugSphere(GetWorld(), FVector(node->Position), 25, 12, FColor::Green, true, -1, 0, 2);
			}
		}	
	}


	
	//Draw all reflections paths in a given order interval
	if (MinOrder != -1 && MaxOrder != -1)
	{
		// Check if the niagara system is valid
		if (SoundRayFX->IsValid())
		{
			// i iterates on the sound rays
			int i = 0;
			// n iterates on the niagara effects
			int n = 0;

			// Getting the front sound ray buffer
			IS_SoundRayArray* SoundRaysFrontBuffer = GetFrontSoundRayBuffer();

			// For all sound rays
			for ( ; i < SoundRaysFrontBuffer->GetNumRays(); i++ )
			{
				// Ray order equals number of points -2 (source and listener are not reflections)
				int rayOrder = SoundRaysFrontBuffer->GetRay(i)->GetNumRayPoints() - 2;

				// Draw the ray if it's in the specified order range
				if (rayOrder >= MinOrder && rayOrder <= MaxOrder)
				{
					UNiagaraComponent* NiagaraComp = nullptr;

					if (NiagaraEffects.Num() > n)
					{
						if (NiagaraEffects[n] != nullptr)
						{
							NiagaraComp = NiagaraEffects[n];
						}
						else
						{
							NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, SoundRayFX, GetActorLocation(), FRotator(1), FVector(1), false, true);
							NiagaraEffects.Insert( NiagaraComp , n );
						}
					}
					else
					{
						NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, SoundRayFX, GetActorLocation(), FRotator(1), FVector(1), false, true);
						NiagaraEffects.Add( NiagaraComp );	
					}

					TArray<FVector> Ray = TArray<FVector>();
					TArray<float> AmplitudesLow = TArray<float>();
					TArray<float> AmplitudesMed = TArray<float>();
					TArray<float> AmplitudesHigh = TArray<float>();

					IS_SoundRay* ray = SoundRaysFrontBuffer->GetRay(i);
					IS_SoundRayPoint* point;

					for ( int j = 0 ; j < ray->GetNumRayPoints() ; j++ )
					{
						point = ray->GetRayPoint(j);

						// Building Niagara FX input parameters
						// Ray position
						Ray.Add(FVector(point->PointPosition));
						AmplitudesLow.Add((point->InAmplitude125 + point->InAmplitude250) / 2);
						AmplitudesLow.Add((point->OutAmplitude125 + point->OutAmplitude250) / 2);
						AmplitudesMed.Add((point->InAmplitude500 + point->InAmplitude1000) / 2);
						AmplitudesMed.Add((point->OutAmplitude500 + point->OutAmplitude1000) / 2);
						AmplitudesHigh.Add((point->InAmplitude2000 + point->InAmplitude4000) / 2);
						AmplitudesHigh.Add((point->OutAmplitude2000 + point->OutAmplitude4000) / 2);
					}

					//UNiagaraComponent* NiagaraComp = NiagaraActor->GetComponentByClass<UNiagaraComponent>();

					if (NiagaraComp != nullptr)
					{
						UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayPosition(NiagaraComp, FName("Ray"), Ray);
						UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloat(NiagaraComp, FName("AmplitudesLow"), AmplitudesLow);
						UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloat(NiagaraComp, FName("AmplitudesMed"), AmplitudesMed);
						UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayFloat(NiagaraComp, FName("AmplitudesHigh"), AmplitudesHigh);
						NiagaraComp->SetFloatParameter(FName("InitialAmplitude"), soundLevel);
						NiagaraComp->Activate(true);
					}

					n++;
				}
			}

			for ( ; n < NiagaraEffects.Num(); n++ )
			{
				if (NiagaraEffects[n] != nullptr)
				{
					NiagaraEffects[n]->DestroyInstance();
					NiagaraEffects.RemoveAt(n);
					n--;
				}
				else
				{
					NiagaraEffects.RemoveAt(n);
					n--;
				}
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Warning: the selected Niagara System is not valid, please select IS_SoundRaysFX"));
		}

		/*
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
		*/
	}
	else
	{
		for ( int n = 0 ; n < NiagaraEffects.Num(); n++ )
		{
			if (NiagaraEffects[n] != nullptr)
			{
				NiagaraEffects[n]->DestroyInstance();
				NiagaraEffects.RemoveAt(n);
				n--;
			}
			else
			{
				NiagaraEffects.RemoveAt(n);
				n--;
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
				DrawDebugPoint(GetWorld(), FVector(node->Position), 15, FColor::Red, true, -1);
				//DrawDebugSphere(GetWorld(), FVector(node->Position), 30, 16, FColor::Red, true, -1, 0, 2);

				// Draws the resulting beam projection on the reflector
				for (IS_ReflectorEdge edge : node->BeamPoints.Edges())
				{
					DrawDebugLine(GetWorld(), FVector(edge.PointA), FVector(edge.PointB), FColor::Red, true, -1, 0, 2);
				}

				// Creates normal of plane on which the reflector lies
				FVector3f planeNormal = FVector3f::CrossProduct(node->BeamPoints.Points()[1] - node->BeamPoints.Points()[0], node->BeamPoints.Points()[2] - node->BeamPoints.Points()[0]);

				// Draws the parent related gizmos
				//Gizmos.color = Color.blue;

				// Highlights the parent IS in blue
				IS* nodeParent = nodes[node->Parent];
				DrawDebugPoint(GetWorld(), FVector(nodeParent->Position), 15, FColor::Blue, true, -1);
				//DrawDebugSphere(GetWorld(), FVector(nodeParent->Position), 30, 16, FColor::Blue, true, -1, 0, 2);

				// Draws parent beam points
				for (IS_ReflectorEdge edge : nodeParent->BeamPoints.Edges())
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
					for (IS_ReflectorEdge edge : nodeParent->BeamPoints.Edges())
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



IS_SoundRayArray* AIS_Source::GetFrontSoundRayBuffer()
{
	if (currentFrontBufferIs1)
		return &SoundRaysBuffer1;
	else
		return &SoundRaysBuffer2;
}



IS_SoundRayArray* AIS_Source::GetBackSoundRayBuffer()
{
	if (currentFrontBufferIs1)
		return &SoundRaysBuffer2;
	else
		return &SoundRaysBuffer1;
}