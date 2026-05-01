#pragma once

class AAIController;
class UBlackboardComponent;

#include <memory>
#include <functional>
#include <vector>
#include <unordered_map>

namespace GameAI::FSM
{
	class Transition;
	class State;
	// Owns and manages States and Transitions.
	// Holds the current State.
	class FSM final
	{
	public:
		FSM() = default;
		~FSM() = default;
		FSM(const FSM&) = delete;
		FSM& operator=(const FSM&) = delete;
		FSM(FSM&&) = delete;
		FSM& operator=(FSM&&) = delete;
		
		void SetBlackboard(UBlackboardComponent* InBlackboard);
		void SetController(AAIController* InController);
		
		State* AddState(std::unique_ptr<State>&& NewState);
		void AddTransition(State* From, State* To, std::function<bool()> Condition);
		
		void Start();
		void Stop();
		void Tick(float DeltaTime);
		
		bool IsRunning() const noexcept;
		
	private:
		std::vector<std::unique_ptr<State>> StatesContainer{};
		// Multiple Transitions PER state
		std::unordered_map<State*, std::vector<Transition>> TransitionsContainer{};
		
		State* CurrentState{ nullptr };
		UBlackboardComponent* Blackboard{ nullptr };
		AAIController* Controller{ nullptr };
		
		bool bRunning{ false };
		
		void ChangeState(State* NewState);
	
	};

}
