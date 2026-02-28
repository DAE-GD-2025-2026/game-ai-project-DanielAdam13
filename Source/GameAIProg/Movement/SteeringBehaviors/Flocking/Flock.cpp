#include "Flock.h"
#include "FlockingSteeringBehaviors.h"
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
{
	Agents.Reserve(FlockSize);
	
	// 1. Create the new behaviors...
	pSeparationBehavior = std::make_unique<Separation>(this);
	pCohesionBehavior = std::make_unique<Cohesion>(this);
	pVelMatchBehavior = std::make_unique<VelocityMatch>(this);
	pSeekBehavior = std::make_unique<Seek>(  );
	pWanderBehavior = std::make_unique<Wander>();
	// SET TARGET OF EVADE
	pEvadeBehavior = std::make_unique<Evade>();
	pEvadeBehavior->SetAgentTarget( pAgentToEvade );
	
	// Then initialize:
	// TODO: Change weights later...
	pBlendedSteering = std::make_unique<BlendedSteering>(std::vector<BlendedSteering::WeightedBehavior>{
	{pCohesionBehavior.get(), 0.f}, {pSeparationBehavior.get(), 0.9f}, 
		{pVelMatchBehavior.get(), 0.5f}, {pSeekBehavior.get(), 0.1f},
	{pWanderBehavior.get(), 0.7f}});
	pPrioritySteering = std::make_unique<PrioritySteering>(std::vector<ISteeringBehavior*>{pEvadeBehavior.get(),
		pBlendedSteering.get()});
	
	// 2. Initialize and spawn flock container's agents
	for (int i{}; i < FlockSize; ++i)
	{
		const double PosRandX{static_cast<double>(FMath::FRandRange(-WorldSize, WorldSize))};
		const double PosRandY{static_cast<double>(FMath::FRandRange(-WorldSize, WorldSize))};
		
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		// Z coordinate might be wrong
		ASteeringAgent* Agent =
			pWorld->SpawnActor<ASteeringAgent>(AgentClass, 
				FVector{PosRandX, PosRandY, 90}, FRotator::ZeroRotator, Params);
		
		if (!Agent)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn flock agent"));
			continue;
		}

		Agent->SetSteeringBehavior(pPrioritySteering.get());

		Agents.Add( Agent );
	}
	
	// for (ASteeringAgent* ag : Agents)
	// {
	// 	ag->SetSteeringBehavior( pPrioritySteering.get() );
	// }
	
	// 3. Initialize memory pool for neighbors
	Neighbors.SetNum( FlockSize );
	NrOfNeighbors = 0;
	
}

Flock::~Flock()
{
 // TODO: Cleanup any additional data
}

void Flock::Tick(float DeltaTime)
{
	// For every agent:
	for (ASteeringAgent* ag : Agents)
	{
		if (!ag)
			continue;
		// Register the neighbors for this agent (-> fill the memory pool with the neighbors for the currently evaluated agent)
		RegisterNeighbors( ag );
		
		// Update the agent (-> the Steering Behaviors use the neighbors in the memory pool)
		ag->Tick( DeltaTime );
		
		// TODO: trim the agent to the world
	}
}

void Flock::RenderDebug()
{
 // TODO: Render all the agents in the flock
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

		// Debug checkbox
		ImGui::Checkbox("Show Neighborhood Debug", &DebugRenderNeighborhood);

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
	
	
	RenderNeighborhood();
}

void Flock::RenderNeighborhood()
{
	if (DebugRenderNeighborhood)
	{
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

			DrawDebugSphere( pWorld,Neighbors[i]->GetActorLocation(), 25.f, 
				8, FColor::Green,false,-1.f,0,2.f);
		}
	}
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
	
	// Sum all Fvector2d of neighbors
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

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
 // TODO: Implement
	
	
}

