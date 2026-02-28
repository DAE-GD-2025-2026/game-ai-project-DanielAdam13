#include "Flock.h"
#include "FlockingSteeringBehaviors.h"
#include "Movement/SteeringBehaviors/SpacePartitioning/SpacePartitioning.h"
#include "Shared/ImGuiHelpers.h"


Flock::Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize,
	float WorldSize,
	ASteeringAgent* const pAgentToEvade,
	bool bTrimWorld)
	: pWorld{pWorld}
	, FlockSize{ FlockSize }
	, NeighRadSqr{ NeighborhoodRadius * NeighborhoodRadius }
	, pAgentToEvade{pAgentToEvade}
	, TrimWorldSize( WorldSize )
	, bShouldTrimWorld( bTrimWorld )
{
	Agents.Reserve(FlockSize);
	
	// 1. Create the new single behaviors...
	pSeparationBehavior = std::make_unique<Separation>(this);
	pCohesionBehavior = std::make_unique<Cohesion>(this);
	pVelMatchBehavior = std::make_unique<VelocityMatch>(this);
	pSeekBehavior = std::make_unique<Seek>(  );
	pWanderBehavior = std::make_unique<Wander>();
	// SET TARGET OF EVADE
	pEvadeBehavior = std::make_unique<Evade>();
	pEvadeBehavior->SetAgentTarget( pAgentToEvade );
	
	// 2. Then initialize the combined behaviors:
	pBlendedSteering = std::make_unique<BlendedSteering>(std::vector<BlendedSteering::WeightedBehavior>{
	{pCohesionBehavior.get(), 0.f}, {pSeparationBehavior.get(), 0.9f}, 
		{pVelMatchBehavior.get(), 0.5f}, {pSeekBehavior.get(), 0.1f},
	{pWanderBehavior.get(), 0.7f}});
	pPrioritySteering = std::make_unique<PrioritySteering>(std::vector<ISteeringBehavior*>{pEvadeBehavior.get(),
		pBlendedSteering.get()});
	
	// 3. Initialize and spawn flock container's agents
	for (int i{}; i < FlockSize; ++i)
	{
		const double PosRandX{static_cast<double>(FMath::FRandRange(-WorldSize, WorldSize))};
		const double PosRandY{static_cast<double>(FMath::FRandRange(-WorldSize, WorldSize))};
		
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		ASteeringAgent* Agent =
			pWorld->SpawnActor<ASteeringAgent>(AgentClass, 
				FVector{PosRandX, PosRandY, 90}, FRotator::ZeroRotator, Params);
		
		if (!Agent)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn flock agent"));
			continue;
		}

		// 4. Set Agent's main behavior - priority
		Agent->SetSteeringBehavior(pPrioritySteering.get());

		Agents.Add( Agent );
		
		// Store the agent pointer for space partitioning
		pPartitionedSpace->AddAgent( *Agent );
		OldPositions[i] = FVector2D(PosRandX, PosRandY);
	}
	
#ifndef GAMEAI_USE_SPACE_PARTITIONING
	// 5. Initialize memory pool for neighbors
	Neighbors.SetNum( Agents.Num() );
	NrOfNeighbors = 0;
#else 
	const float worldSize = TrimWorldSize * 2.f;
	
	pPartitionedSpace = std::make_unique<CellSpace>(pWorld, worldSize, worldSize, 
		NrOfCellsX, NrOfCellsX, FlockSize);
	
	OldPositions.SetNum( FlockSize );
#endif
	
}

Flock::~Flock()
{
	for (ASteeringAgent* Agent : Agents)
	{
		if (Agent && !Agent->IsPendingKillPending())
		{
			Agent->Destroy(  );
		}
	}
	Agents.Empty(  );
}

void Flock::Tick(float DeltaTime)
{
#ifndef GAMEAI_USE_SPACE_PARTITIONING
	// For every agent:
	for (ASteeringAgent* ag : Agents)
	{
		if (!ag)
			continue;
		// Register the neighbors for this agent (-> fill the memory pool with the neighbors for the currently evaluated agent)
		RegisterNeighbors( ag );
		
		// Update the agent (-> the Steering Behaviors use the neighbors in the memory pool)
		ag->Tick( DeltaTime );
		
		TrimAgentToWorld( ag );
	}
#else
	for (size_t i{}; i < Agents.Num(); ++i)
	{
		auto* CurrentAgent = Agents[i];
		const FVector2D NewPos{FVector2D(CurrentAgent->GetActorLocation())};
		
		pPartitionedSpace->UpdateAgentCell( *CurrentAgent, OldPositions[i] );
		OldPositions[i] = NewPos;
	}
#endif
}

void Flock::RenderDebug()
{
	RenderNeighborhood();
}

void Flock::ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize)
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		bool bWindowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Gameplay Programming", &bWindowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scroll wheel: zoom cam.");
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Flocking");
		ImGui::Spacing();
		

		// ImGUI CHECKBOXES for debug rendering here
		ImGui::Text("Flocking");
		ImGui::Spacing();
		
		ImGui::Checkbox("Show Neighborhood Debug", &DebugRenderNeighborhood);
		ImGui::Checkbox("Show Render Partitions", &DebugRenderPartitions);
		ImGui::Checkbox("Show Steering", &DebugRenderSteering);

		ImGui::Spacing();
		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

		// SLIDERS FOR BEHAVIOR WEIGHTS
		auto& Weights = pBlendedSteering->GetWeightedBehaviorsRef();
		
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Cohesion",Weights[0].Weight,
		0.f, 1.f,[&](float v)
				{
					Weights[0].Weight = v;
				},"%.2f");
 
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Separation",Weights[1].Weight,
		0.f, 1.f,[&](float v)
				{
					Weights[1].Weight = v;
				}, "%.2f");
		
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Velocity Match",Weights[2].Weight,
		0.f, 1.f,[&](float v)
				{
					Weights[2].Weight = v;
				},"%.2f");
		
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",Weights[3].Weight,
		0.f, 1.f,[&](float v)
				{
					Weights[3].Weight = v;
				},"%.2f");
				
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander",Weights[4].Weight,
		0.f, 1.f,[&](float v)
				{
					Weights[4].Weight = v;
				},"%.2f");
		
		//End
		ImGui::End();
	}
#pragma endregion
#endif
}

void Flock::RenderNeighborhood()
{
	if (DebugRenderNeighborhood)
	{
#ifndef GAMEAI_USE_SPACE_PARTITIONING
		if (Agents.Num() == 0)
			return;
	
		ASteeringAgent* FirstAgent { Agents[0]};
		if (!FirstAgent)
			return;
	
		// Recompute so it is up to date
		RegisterNeighbors(FirstAgent);
	
		const FVector Center{ FirstAgent->GetActorLocation()};
	
		// 1. Draw neighborhood radius
		DrawDebugCircle(
			pWorld, Center,NeighborhoodRadius,32, FColor::Yellow,false, -1.f,0,
			2.f,FVector(1,0,0),FVector(0,1,0),false);
	
		for (int i = 0; i < NrOfNeighbors; ++i)
		{
			if (!Neighbors[i])
				continue;

			DrawDebugSphere( pWorld,Neighbors[i]->GetActorLocation(), 35.f, 
				8, FColor::Green,false,-1.f,0,2.f);
		}
#else
		
#endif
		
	}
}

void Flock::TrimAgentToWorld(ASteeringAgent* Agent) const
{
	if (!bShouldTrimWorld || !Agent)
		return;
	
	FVector Pos{ Agent->GetActorLocation() };
	
	const float Min{ -TrimWorldSize };
	const float Max{ TrimWorldSize };
	
	if (Pos.X < Min) 
		Pos.X = Max;
	else if (Pos.X > Max) 
		Pos.X = Min;

	if (Pos.Y < Min) 
		Pos.Y = Max;
	else if (Pos.Y > Max) 
		Pos.Y = Min;

	Agent->SetActorLocation(Pos);
}

#ifndef GAMEAI_USE_SPACE_PARTITIONING
void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
	NrOfNeighbors = 0;
	
	for (ASteeringAgent* ag : Agents)
	{
		if (!ag)
			continue;
		// If same agent -> skip
		if (ag == pAgent)
			continue;
		
		const float Distance{static_cast<float>( FVector::DistSquared( 
			pAgent->GetActorLocation(), ag->GetActorLocation() ))};
		
		
		if (Distance < NeighRadSqr)
		{
			Neighbors[NrOfNeighbors] = ag;
			++NrOfNeighbors;
		}
	}
}
#endif

FVector2D Flock::GetAverageNeighborPos() const
{
	FVector2D AvgPosition{ FVector2D::ZeroVector };
	
	if (NrOfNeighbors == 0)
		return AvgPosition;
	
	// Sum all FVector2D of neighbors
	for (int i{}; i< NrOfNeighbors; ++i)
	{
		const FVector NeighborPos{Neighbors[i]->GetActorLocation()};
		AvgPosition += FVector2D(NeighborPos.X, NeighborPos.Y);
	}
	// Get average
	AvgPosition /= NrOfNeighbors;
	
	return AvgPosition;
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	FVector2D AvgVelocity { FVector2D::ZeroVector};

	if (NrOfNeighbors == 0)
		return AvgVelocity;
	
	// Sum all velocity of neighbors
	for (int i{}; i< NrOfNeighbors; ++i)
	{
		if (!Neighbors[i])
			continue;
		
		AvgVelocity += Neighbors[i]->GetLinearVelocity();
	}
	// Get average
	AvgVelocity /= NrOfNeighbors;
	
	return AvgVelocity;
}

void Flock::SetTarget_Seek(FSteeringParams const& Target) const
{
	pSeekBehavior->SetTarget( Target );
}

