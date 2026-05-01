#include "PatrolState.h"

#include "../FSMPerceptionHelper.h"

#include "AIController.h"
#include "VectorTypes.h"
#include "Movement/SteeringBehaviors/SteeringAgent.h"
#include "Movement/SteeringBehaviors/PathFollow/PathFollowSteeringBehavior.h"

using namespace GameAI::FSM;

PatrolState::PatrolState(const std::vector<FVector2D>& InPatrolPath)
	:PatrolPath( InPatrolPath )
{
}

PatrolState::~PatrolState()
{
	delete PathBehavior;
}

void PatrolState::OnEnter()
{
	// Set agent steering behavior
	if (auto* Agent = Cast<ASteeringAgent>(GetController()->GetPawn()))
	{
		PathBehavior->SetPath( PatrolPath );
		Agent->SetSteeringBehavior( PathBehavior );
	}
}

void PatrolState::OnUpdate(float DeltaTime)
{
	GameAi::FSM::UpdateTargetVisibility( GetController(), GetBlackboard(), {} );
	
	if (auto* Agent =Cast<ASteeringAgent>(GetController()->GetPawn()))
	{
		const FVector2D Pos{ Agent->GetPosition() };
		const FVector2D LastPos{ PatrolPath.empty() ? FVector2D::ZeroVector : PatrolPath.back() };
		const float Radius{ Agent->GetCapsuleRadius() };
		
		if (!PatrolPath.empty() && UE::Geometry::DistanceSquared( Pos, LastPos ) < Radius * Radius)
		{
			// Reached the end - reset path
			PathBehavior->SetPath( PatrolPath );
		}
	}
}

void PatrolState::OnExit()
{
	if (auto* Agent = Cast<ASteeringAgent>(GetController()->GetPawn()))
	{
		Agent->SetSteeringBehavior( nullptr );
	}
}
