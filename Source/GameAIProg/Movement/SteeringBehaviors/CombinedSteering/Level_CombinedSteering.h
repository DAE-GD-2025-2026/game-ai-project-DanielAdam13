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
	//Datamembers
	bool UseMouseTarget = false;
	bool CanDebugRender = false;
	
	enum class EBehaviors
	{
		Seek,
		Wander
	};
	
	struct FCombinedAgent
	{
		ASteeringAgent* Agent{};
		ISteeringBehavior* Behavior{};
	};
	
	std::vector<std::unique_ptr<FCombinedAgent>> CombinedAgents{};
	
	std::unique_ptr<BlendedSteering> pTemplateSteering;
	
	std::unique_ptr<Seek> SeekTemplate{};
	std::unique_ptr<Wander> WanderTemplate{};
	std::vector<BlendedSteering::WeightedBehavior> TemplateBehaviors{};
	
	void AddAgent();
	
	void UpdateTarget(const FCombinedAgent* Agent) const;
};
