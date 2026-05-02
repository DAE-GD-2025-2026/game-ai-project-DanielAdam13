#include "ChaseState.h"

#include "../FSMPerceptionHelper.h"
#include "../BlackboardKeys.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"
#include "Movement/SteeringBehaviors/SteeringAgent.h"
#include "Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

using namespace  GameAI::FSM;

ChaseState::ChaseState()
	:SeekBehavior( new Seek() )
{
}

ChaseState::~ChaseState()
{
	delete SeekBehavior;
}

void ChaseState::OnEnter()
{
	if (auto* Agent = Cast<ASteeringAgent>(GetController()->GetPawn()))
	{
		Agent->SetSteeringBehavior(SeekBehavior);
	}
}

void ChaseState::OnUpdate(float DeltaTime)
{
	GameAI::FSM::UpdateTargetVisibility(GetController(), GetBlackboard(), {});
	
	// Re-target the seek every frame at the player's current location
	if (AActor* Target = Cast<AActor>(GetBlackboard()->GetValueAsObject(BBKeys::TargetActor)))
	{
		const FVector Loc{ Target->GetActorLocation() };
		const FTargetData TD{ FVector2D{Loc.X, Loc.Y} };
		SeekBehavior->SetTarget(TD);
	}
}

void ChaseState::OnExit()
{
	if (auto* Agent = Cast<ASteeringAgent>(GetController()->GetPawn()))
	{
		Agent->SetSteeringBehavior(nullptr);
	}
}
