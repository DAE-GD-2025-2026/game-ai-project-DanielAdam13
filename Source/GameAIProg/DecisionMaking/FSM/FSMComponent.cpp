#include "FSMComponent.h"

#include "AIController.h"
#include "FSM.h"
#include "State.h"
#include "Transition.h"


UFSMComponent::UFSMComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Create FSM unique_ptr
	FSMInstance = std::make_unique<GameAI::FSM::FSM>();
}

GameAI::FSM::State* UFSMComponent::AddState(std::unique_ptr<GameAI::FSM::State>&& NewState)
{
	if (!FSMInstance)
		return nullptr;
	
	return FSMInstance->AddState( std::move( NewState ) );
}

void UFSMComponent::AddTransition(GameAI::FSM::State* From, GameAI::FSM::State* To, std::function<bool()> EvalFunc) const
{
	if (FSMInstance)
		FSMInstance->AddTransition( From, To, std::move(EvalFunc) );
		
}

// Called when the game starts
void UFSMComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (AAIController* AI = Cast<AAIController>( GetOwner() ))
	{
		FSMInstance->SetController( AI );
	}
}


// Called every frame
void UFSMComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (FSMInstance)
		FSMInstance->Tick( DeltaTime );
}

void UFSMComponent::StartLogic()
{
	Super::StartLogic();

	if (FSMInstance) 
		FSMInstance->Start();
}

void UFSMComponent::StopLogic(const FString& Reason)
{
	if (FSMInstance) 
		FSMInstance->Stop();
}

bool UFSMComponent::IsRunning() const
{
	return FSMInstance && FSMInstance->IsRunning();
}

void UFSMComponent::SetBlackboard(UBlackboardComponent* InBlackboard)
{
	if (FSMInstance)
		FSMInstance->SetBlackboard( InBlackboard );
}

