// Fill out your copyright notice in the Description page of Project Settings.


#include "Level_FSM.h"

#include "BlackboardKeys.h"
#include "FSMComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DecisionMaking/GameAIController.h"
#include "FSMStates/ChaseState.h"
#include "FSMStates/PatrolState.h"
#include "FSMStates/SearchState.h"
#include "Kismet/GameplayStatics.h"

#include "Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

ALevel_FSM::ALevel_FSM()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_FSM::BeginPlay()
{
	Super::BeginPlay();
	
	// --------- THIEF ------------
	Thief = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, 
	FVector{800.f,800.f,90.f}, FRotator::ZeroRotator);
	
	ThiefArriveBehavior = new Arrive();
	ThiefArriveBehavior->SetTargetRadius( 40.f );
	Thief->SetSteeringBehavior( ThiefArriveBehavior );
	
	
	// --------- GUARD ------------
	// Spawn the guard
	Agent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, 
	FVector{0.f,0.f,90.f}, FRotator::ZeroRotator);
	Agent->SetDebugRenderingEnabled(false);
	
	// Get the controller and FSM compt off the agent
	auto* AIController{ Cast<AGameAIController>(Agent->GetController()) };
	if (!AIController) 
		return;
	auto* FSM{ Cast<UFSMComponent>(AIController->GetBrainComponent()) };
	if (!FSM) 
		return;
	UBlackboardComponent* BB{ AIController->GetBlackboardComponent() };
	if (!BB) 
		return;
	
	// Stash the player on the B
	BB->SetValueAsObject( GameAI::FSM::BBKeys::TargetActor, Thief );
	
	// Define the patrol path
	std::vector<FVector2D> PatrolPath = {
		FVector2D{   200.f,    200.f},
		FVector2D{ 500.f,    200.f},
		FVector2D{ 500.f,  500.f},
		FVector2D{   200.f,  500.f},
	};
	
	// ADD THE STATES
	using namespace  GameAI::FSM;
	State* Patrol = FSM->AddState(std::make_unique<PatrolState>(PatrolPath));
	State* Chase  = FSM->AddState(std::make_unique<ChaseState>());
	State* Search = FSM->AddState(std::make_unique<SearchState>());
	
	// ADD THE TRANSITIONS
	constexpr float SearchTimeout{ 5.f };
	
	auto IsTargetVisible{ [BB]() -> bool
	{
		return BB->GetValueAsBool( BBKeys::IsTargetVisible );
	}};
	auto IsTargetNotVisible = [BB]() -> bool
	{
		return !BB->GetValueAsBool(BBKeys::IsTargetVisible);
	};
	auto IsSearchingTooLong = [BB, SearchTimeout]() -> bool
	{
		return BB->GetValueAsFloat(BBKeys::SearchTimer) >= SearchTimeout;
	};
	
	// Add the transitions to FSM...
	FSM->AddTransition( Patrol, Chase, IsTargetVisible );
	FSM->AddTransition( Chase, Search, IsTargetNotVisible );
	FSM->AddTransition( Search, Chase, IsTargetVisible );
	FSM->AddTransition( Search, Patrol, IsSearchingTooLong );
	
	AIController->RunFiniteStateMachine();
	
}

void ALevel_FSM::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	delete ThiefArriveBehavior;
	ThiefArriveBehavior = nullptr;
	Super::EndPlay( EndPlayReason );
}

// Called every frame
void ALevel_FSM::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (Thief && ThiefArriveBehavior)
	{
		ThiefArriveBehavior->SetTarget( MouseTarget );
	}
}

