#include "FSMPerceptionHelper.h"

#include "BlackboardKeys.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

namespace GameAi::FSM
{
	bool UpdateTargetVisibility(AAIController* Controller, UBlackboardComponent* BlackboardComponent,
		const PerceptionParams& Params)
	{
		if (!Controller || !BlackboardComponent)
			return false;
		
		APawn* Self{ Controller->GetPawn() };
		const AActor* Target{ Cast<AActor>(
			BlackboardComponent->GetValueAsObject(GameAI::FSM::BBKeys::TargetActor)) };
		if (!Self || !Target)
		{
			BlackboardComponent->SetValueAsBool( GameAI::FSM::BBKeys::IsTargetVisible, false );
			return false;
		}
		
		const FVector SelfLocation{ Self->GetActorLocation() };
		const FVector TargetLocation{ Target->GetActorLocation() };
		
		// 1. RADIUS
		const float DistSqr{ static_cast<float>(FVector::Dist( SelfLocation, TargetLocation )) };
		if (DistSqr > Params.DetectionRadius * Params.DetectionRadius)
		{
			BlackboardComponent->SetValueAsBool( GameAI::FSM::BBKeys::IsTargetVisible, false );
			return false;
		}
		
		// 2. Line-of-sight (raycast)
		FHitResult Hit;
		FCollisionQueryParams Query{SCENE_QUERY_STAT(FSMPerception), false, Self};
		Query.AddIgnoredActor( Target );
		
		const FVector Start{ SelfLocation + FVector(0, 0, Params.LineOfSightHeightTrace) };
		const FVector End{ TargetLocation + FVector(0, 0, Params.LineOfSightHeightTrace) };
		
		const bool bBlocked{ Controller->GetWorld()->LineTraceSingleByChannel(
			Hit, Start, End, ECC_Visibility, Query) };
		const bool bVisible{ !bBlocked };
		
		BlackboardComponent->SetValueAsBool(GameAI::FSM::BBKeys::IsTargetVisible, bVisible);
		if (bVisible)
		{
			BlackboardComponent->SetValueAsVector(GameAI::FSM::BBKeys::LastSeenLocation, TargetLocation);
		}
		
		DrawDebugLine(Controller->GetWorld(), Start, End,
					  bVisible ? FColor::Green : FColor::Red, false, 0.1f);
		
		return bVisible;
	}
}
