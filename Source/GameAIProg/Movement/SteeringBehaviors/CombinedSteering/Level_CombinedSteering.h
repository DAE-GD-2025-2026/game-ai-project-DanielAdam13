#pragma once

#include "CoreMinimal.h"
#include <memory>
#include "CombinedSteeringBehaviors.h"
#include "GameAIProg/Shared/Level_Base.h"
#include "GameAIProg/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"
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
	
	enum class Behaviors
	{
		Seek,
		Wander
	};
	
	struct CombinedAgent
	{
		ASteeringAgent* Agent{ nullptr };
		std::unique_ptr<BlendedSteering> Behavior{ nullptr };
	};
	
	std::vector<CombinedAgent> CombinedAgents{};
	
	std::unique_ptr<BlendedSteering> pTemplateSteering{};
	
	std::unique_ptr<Seek> SeekTemplate{};
	std::unique_ptr<Wander> WanderTemplate{};
	
	//std::unique_ptr<BlendedSteering> m_pBlendedSteering;
	
	void AddAgent();
	
	void UpdateTarget(CombinedAgent& agent);
};
