#pragma once

#include "CoreMinimal.h"
#include <memory>
#include "CombinedSteeringBehaviors.h"
#include "GameAIProg/Shared/Level_Base.h"
#include "GameAIProg/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "Level_CombinedSteering.generated.h"

UCLASS()
class GAMEAIPROG_API ALevel_CombinedSteering : public ALevel_Base
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALevel_CombinedSteering();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;

private:
	// Data members
	bool UseMouseTarget = false;
	bool CanDebugRender = false;
	
	enum class EBehaviors
	{
		Blended,
		Priority,
		Wanderer
	};
	
	struct FCombinedAgent
	{
		ASteeringAgent* Agent{};
		ISteeringBehavior* Behavior{}; // Because you can't Agent.pBehavior...
	};
	
	std::vector<std::unique_ptr<FCombinedAgent>> CombinedAgents{};
	
	// Blended Steering:
	std::unique_ptr<BlendedSteering> pTemplateBlendedSteering;
	
	std::unique_ptr<Seek> SeekBlendedTemplate{};
	std::unique_ptr<Wander> WanderBlendedTemplate{};
	
	void AddAgent(EBehaviors behaviorType);
	void UpdateTargetToMouse(const FCombinedAgent* Agent) const;
	
	// Priority Steering:
	std::unique_ptr<PrioritySteering> pTemplatePrioritySteering;
	std::unique_ptr<Evade> EvadePriorityTemplate{ nullptr };
	std::unique_ptr<Wander> WanderPriorityTemplate{ nullptr };
	
	std::unique_ptr<FCombinedAgent> WandererAgent{ nullptr };
	
};
