#include "Transition.h"

using namespace GameAI::FSM;

Transition::Transition(State* InFrom, State* InTo, std::function<bool()> InCondition)
	:m_From( InFrom ),
	m_To( InTo ),
	m_Condition( InCondition )
{
}

State* Transition::GetFrom() const noexcept
{
	return m_From;
}

State* Transition::GetTo() const noexcept
{
	return m_To;
}

bool Transition::Evaluate() const noexcept
{
	return m_Condition && m_Condition();
}
