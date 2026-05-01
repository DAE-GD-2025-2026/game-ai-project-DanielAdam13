#pragma once
#include "../State.h"

#include <vector>

class PathFollow;

namespace GameAI::FSM
{
	class PatrolState final : public State
	{
	public:
		PatrolState(const std::vector<FVector2D>& InPatrolPath);
		virtual ~PatrolState() override;
		
		virtual void OnEnter() override;
		virtual void OnUpdate(float DeltaTime) override;
		virtual void OnExit() override;
		
	private:
		std::vector<FVector2D> PatrolPath{};
		PathFollow* PathBehavior{ nullptr };
		
	};

}
