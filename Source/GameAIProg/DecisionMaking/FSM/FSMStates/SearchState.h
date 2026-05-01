#pragma once

#include "CoreMinimal.h"
#include "../State.h"

class Arrive;
class Wander;

namespace GameAI::FSM
{
	class SearchState final : public State
	{
	public:
		SearchState();
		virtual ~SearchState() override;
		
		virtual void OnEnter() override;
		virtual void OnUpdate(float DeltaTime) override;
		virtual void OnExit() override;
		
	private:
		Arrive* ArriveBehavior{ nullptr }; // owned
		Wander* WanderBehavior{ nullptr }; // owned
		bool bReachedLastSeen{ false };
		
	};
}


