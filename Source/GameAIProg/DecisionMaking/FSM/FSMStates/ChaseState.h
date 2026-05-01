#pragma once

#include "CoreMinimal.h"
#include "../State.h"

class Seek;

namespace GameAI::FSM
{
	class ChaseState final : public State
	{
	public:
		ChaseState();
		virtual ~ChaseState() override;
		
		virtual void OnEnter() override;
		virtual void OnUpdate(float DeltaTime) override;
		virtual void OnExit() override;

	private:
		Seek* SeekBehavior{ nullptr }; // owned
		
	};
}

