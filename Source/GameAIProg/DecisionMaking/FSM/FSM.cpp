#include "FSM.h"

#include "State.h"
#include "Transition.h"

using namespace GameAI::FSM;

void FSM::SetBlackboard(UBlackboardComponent* InBlackboard)
{
	Blackboard = InBlackboard;
}

void FSM::SetController(AAIController* InController)
{
	Controller = InController;
}

State* FSM::AddState(std::unique_ptr<State>&& NewState)
{
	if (!NewState) 
		return nullptr;
	
	NewState->SetBlackboard( Blackboard );
	NewState->SetController( Controller );
	
	State* RawState{ NewState.get() };
	StatesContainer.push_back( std::move(NewState) );
	return RawState;
}

void FSM::AddTransition(State* From, State* To, std::function<bool()> Condition)
{
	if (!From || !To || !Condition)
		return;
	
	TransitionsContainer[From].emplace_back( From, To, Condition );
}

void FSM::Start()
{
	if (bRunning || StatesContainer.empty())
		return;
	
	bRunning = true;
	
	// Start first state
	ChangeState( StatesContainer.front().get() );
}

void FSM::Stop()
{
	if (!bRunning)
		return;
	
	// Exit current state as soon as possible
	if (CurrentState)
		CurrentState->OnExit();
	
	CurrentState = nullptr;
	bRunning = false;
}

void FSM::Tick(float DeltaTime)
{
	if (!bRunning || !CurrentState)
		return;
	
	CurrentState->OnUpdate( DeltaTime );
	
	// Evaluate ongoing transitions:
	auto It{TransitionsContainer.find( CurrentState )};
	if (It == TransitionsContainer.end())
		return;
	
	// First one to be evaluated true
	for (const Transition& t : It->second)
	{
		if (t.Evaluate())
		{
			ChangeState( t.GetTo() );
			break;
		}
	}
}

bool FSM::IsRunning() const noexcept
{
	return bRunning;
}

void FSM::ChangeState(State* NewState)
{
	if (CurrentState == NewState)
		return;
	
	// 1.
	if (CurrentState) 
		CurrentState->OnExit();
	// 2.
	CurrentState = NewState;
	// 3.
	CurrentState->OnEnter();
}
