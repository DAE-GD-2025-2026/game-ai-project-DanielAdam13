#pragma once

#include <Movement/SteeringBehaviors/SteeringHelpers.h>
#include "Kismet/KismetMathLibrary.h"

class ASteeringAgent;

// SteeringBehavior base, all steering behaviors should derive from this.
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	// Override to implement your own behavior
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent);

	void SetTarget(const FTargetData& NewTarget) { Target = NewTarget; }
	
	template<class T, std::enable_if_t<std::is_base_of_v<ISteeringBehavior, T>>* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	FTargetData Target;
};

// Your own SteeringBehaviors should follow here...
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() override = default;

	// Seek Behaviour
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;

protected:
};

class Flee : public ISteeringBehavior
{
public:
	Flee() = default;
	virtual ~Flee() override = default;

	// Flee Behaviour
	virtual SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& Agent) override;

protected:
};

class Arrive : public Seek
{
public:
	Arrive() = default;
	virtual ~Arrive() override = default;

	// Arrive Behaviour
	virtual SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& Agent) override;
protected:
	FVector2D m_MaxLinearVelocity{};
	
	const float m_OuterRadius{ 300.f };
	const float m_InnerRadius{ 20.f };

private:
};

class Face : public ISteeringBehavior
{
public:
	Face() = default;
	virtual ~Face() override = default;

	// Face Behaviour
	virtual SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& Agent) override;
protected:
};

class Wander : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() override = default;

	// Wander Logic
	virtual SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& Agent) override;


protected:
	const float m_OffsetDistance{ 200.f };
	const float m_WanderRadius{ 120.f };

	const float m_MaxAngleChange{ FMath::DegreesToRadians(45.f) };

	float m_ChangeTimer{ 0.f };
	const float m_MaxTargetChangeInterval{ 0.2f };

	float m_OnSwitchAngle{};
	float m_LastOnSwitchAngle{};

	virtual FVector2D GetRandomPointInCircle(const FVector2D& circleCenter);
	
private:
};

