#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BlackboardComponent.h"

class AAIController;
class UBlackboardComponent;
class AActor;

namespace GameAI::FSM
{
	// CUSTOM values
	struct PerceptionParams
	{
		float DetectionRadius{ 600.f };
		float LineOfSightHeightTrace{ 50.f };
	};
	
	// Free function for radius + line-of-sight calculation.
	// Makes use of the custom blackboard - the BlackboardKeys.h
	bool UpdateTargetVisibility(AAIController* Controller, UBlackboardComponent* BlackboardComponent,
		const PerceptionParams& Params);
}
