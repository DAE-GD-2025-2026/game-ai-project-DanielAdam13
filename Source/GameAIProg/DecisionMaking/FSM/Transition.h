#pragma once

#include <functional>

namespace GameAI::FSM
{
	class State;
	
	class Transition
	{
	public:
		Transition(State* InFrom, State* InTo, std::function<bool()> InCondition);
		
		State* GetFrom() const noexcept;
		State* GetTo() const noexcept;
		bool Evaluate() const noexcept;
		
	private:
		State* m_From{ nullptr };
		State* m_To{ nullptr };
		std::function<bool()> m_Condition;
	
	};
}

