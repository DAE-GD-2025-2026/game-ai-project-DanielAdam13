#include "SearchState.h"

#include "../FSMPerceptionHelper.h"
#include "../BlackboardKeys.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Movement/SteeringBehaviors/SteeringAgent.h"
#include "Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

using namespace GameAI::FSM;

SearchState::SearchState()
	:ArriveBehavior( new Arrive() ),
	WanderBehavior( new Wander() )
{
	ArriveBehavior->SetTargetRadius( 40.f );
}

SearchState::~SearchState()
{
	delete ArriveBehavior;
	delete WanderBehavior;
}

void SearchState::OnEnter()
{
	bReachedLastSeen = false;
	
	// Modify search value in BLACKBOARD
	GetBlackboard()->SetValueAsFloat(BBKeys::SearchTimer, 0.f);
	
	// Head to the last-seen location first using the BLACKBOARD LastSeenLoc value
	const FVector LastSeen{ GetBlackboard()->GetValueAsVector(BBKeys::LastSeenLocation) };
	const FTargetData TD{ FVector2D{LastSeen.X, LastSeen.Y} };
	ArriveBehavior->SetTarget(TD);
	
	if (auto* Agent = Cast<ASteeringAgent>(GetController()->GetPawn()))
	{
		Agent->SetSteeringBehavior(ArriveBehavior);
	}
}

void SearchState::OnUpdate(float DeltaTime)
{
	// Update Search timer in BLACKBOARD
	const float SearchTimer{ GetBlackboard()->GetValueAsFloat( BBKeys::SearchTimer ) };
	GetBlackboard()->SetValueAsFloat( BBKeys::SearchTimer, SearchTimer + DeltaTime );
	
	GameAI::FSM::UpdateTargetVisibility( GetController(), GetBlackboard(), {} );
	
	// Wander:
	if (!bReachedLastSeen)
	{
		if (auto* Agent = Cast<ASteeringAgent>(GetController()->GetPawn()))
		{
			const FVector LastSeenLoc{ GetBlackboard()->GetValueAsVector(BBKeys::LastSeenLocation)};
			const FVector2D LastSeen2D{ LastSeenLoc.X, LastSeenLoc.Y };
			constexpr float ArriveRadius{ 60.f };

			if (FVector2D::DistSquared(Agent->GetPosition(), LastSeen2D)
				< ArriveRadius * ArriveRadius)
			{
				bReachedLastSeen = true;
				Agent->SetSteeringBehavior(WanderBehavior);
			}
		}
	}
}

void SearchState::OnExit()
{
	if (auto* Agent = Cast<ASteeringAgent>(GetController()->GetPawn()))
	{
		Agent->SetSteeringBehavior(nullptr);
	}
}


