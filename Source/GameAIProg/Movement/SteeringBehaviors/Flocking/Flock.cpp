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
	Agents.SetNum(FlockSize);
	
    // 1. Initialize and spawn flock container's agents
	for (int i{}; i < FlockSize; ++i)
	{
		const double PosRandX{static_cast<double>(FMath::FRandRange(-WorldSize, WorldSize))};
		const double PosRandY{static_cast<double>(FMath::FRandRange(-WorldSize, WorldSize))};
		
		// Z coordinate might be wrong
		Agents[i] = pWorld->SpawnActor<ASteeringAgent>(AgentClass, 
			FVector{PosRandX, PosRandY, 90},FRotator::ZeroRotator);
	}
	
	// 2. Create the new behaviors...
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
	{pCohesionBehavior.get(), 0.f}, {pSeparationBehavior.get(), 0.f}, 
		{pVelMatchBehavior.get(), 0.f}, {pSeekBehavior.get(), 0.f},
	{pWanderBehavior.get(), 0.f}});
	pPrioritySteering = std::make_unique<PrioritySteering>(std::vector<ISteeringBehavior*>{pEvadeBehavior.get(),
		pBlendedSteering.get()});
	
	for (ASteeringAgent* ag : Agents)
	{
		ag->SetSteeringBehavior( pPrioritySteering.get() );
	}
	
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
		// Register the neighbors for this agent (-> fill the memory pool with the neighbors for the currently evaluated agent)
		RegisterNeighbors( ag );
		
		// Update the agent (-> the steeringbehaviors use the neighbors in the memory pool)
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

  // TODO: implement ImGUI checkboxes for debug rendering here

		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

  // TODO: implement ImGUI sliders for steering behavior weights here
		//End
		ImGui::End();
	}
#pragma endregion
#endif
}

void Flock::RenderNeighborhood()
{
 // TODO: Debugrender the neighbors for the first agent in the flock
}

#ifndef GAMEAI_USE_SPACE_PARTITIONING
void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
	NrOfNeighbors = 0;
	
	for (ASteeringAgent* ag : Agents)
	{
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
		 AvgPosition += static_cast<FVector2D>(Neighbors[i]->GetActorLocation());
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

