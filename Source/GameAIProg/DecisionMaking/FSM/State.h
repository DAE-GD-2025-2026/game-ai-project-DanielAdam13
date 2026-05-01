#pragma once

class AAIController;
class UBlackboardComponent;

namespace GameAI::FSM
{
	// Base State class
	class State
	{
	public:
		State() = default;
		virtual ~State() = default;
		State(const State&) = delete;
		State& operator=(const State&) = delete;
		State(State&&) = delete;
		State& operator=(State&&) = delete;
		
		virtual void OnEnter() {};
		virtual void OnUpdate(float DeltaTime) {};
		virtual void OnExit() {};
		
		void SetBlackboard(UBlackboardComponent* InBlackboard) { Blackboard = InBlackboard; }
		void SetController(AAIController* InController) { Controller = InController; }
		
	protected:
		UBlackboardComponent* GetBlackboard() const { return Blackboard; }
		AAIController* GetController() const { return Controller; }
		
	private:
		UBlackboardComponent* Blackboard{ nullptr};
		AAIController* Controller{nullptr};
		
	};
}

