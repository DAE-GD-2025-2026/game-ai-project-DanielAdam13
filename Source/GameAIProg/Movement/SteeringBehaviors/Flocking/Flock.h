#pragma once

// Toggle this define to enable/disable spatial partitioning
//#define GAMEAI_USE_SPACE_PARTITIONING

#include "FlockingSteeringBehaviors.h"
#include "Movement/SteeringBehaviors/SteeringAgent.h"
#include "Movement/SteeringBehaviors/SteeringHelpers.h"
#include "Movement/SteeringBehaviors/CombinedSteering/CombinedSteeringBehaviors.h"
#include <memory>
#include "imgui.h"

// For Spacial Partitioning
class CellSpace;

class Flock final
{
public:
	Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize = 10, 
	float WorldSize = 100.f, 
	ASteeringAgent* const pAgentToEvade = nullptr, 
	bool bTrimWorld = false);

	~Flock();

	void Tick(float DeltaTime);
	void RenderDebug();
	void ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize);

// For Spacial Partitioning
	const TArray<ASteeringAgent*>& GetNeighbors() const;
	int GetNrOfNeighbors() const;
// No spacial partitioning
	void RegisterNeighbors(const ASteeringAgent* const Agent);


	FVector2D GetAverageNeighborPos() const;
	FVector2D GetAverageNeighborVelocity() const;

	void SetTarget_Seek(FSteeringParams const & Target) const;

private:
	// For debug rendering purposes
	UWorld* pWorld{nullptr};
	
	int FlockSize{ 600 };
	TArray<ASteeringAgent*> Agents{};
	
	// For Spacial Partitioning
	std::unique_ptr<CellSpace> pPartitionedSpace{};
	const int NrOfCellsX{ 15 };
	TArray<FVector2D> OldPositions{};
	
	// No spacial partitioning
	TArray<ASteeringAgent*> Neighbors{};
	
	
	const float NeighborhoodRadius{400.f};
	const float NeighRadSqr;
	int NrOfNeighbors{0};

	ASteeringAgent* pAgentToEvade{nullptr};
	
	//Steering Behaviors
	std::unique_ptr<Separation> pSeparationBehavior{};
	std::unique_ptr<Cohesion> pCohesionBehavior{};
	std::unique_ptr<VelocityMatch> pVelMatchBehavior{};
	std::unique_ptr<Seek> pSeekBehavior{};
	std::unique_ptr<Wander> pWanderBehavior{};
	std::unique_ptr<Evade> pEvadeBehavior{};
	
	std::unique_ptr<BlendedSteering> pBlendedSteering{};
	std::unique_ptr<PrioritySteering> pPrioritySteering{};

	// UI and rendering
	bool DebugRenderSteering{false};
	bool DebugRenderNeighborhood{false};
	bool DebugRenderPartitions{false};

	void RenderNeighborhood();
	
	float TrimWorldSize{0.f};
	bool bShouldTrimWorld{false};
	
	void TrimAgentToWorld(ASteeringAgent* Agent) const;
	
	bool SpacialPartitioningActive{true};
};
