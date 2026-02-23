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

	virtual void SetTarget(const FTargetData& NewTarget) { Target = NewTarget; }
	
	template<class T, std::enable_if_t<std::is_base_of_v<ISteeringBehavior, T>>* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

	void SetAgentTarget(ASteeringAgent* OtherAgent) 
	{
		m_TargetAgent = OtherAgent;
	}

protected:
	FTargetData Target;

	FVector2D currentLinearVelocity;

	ASteeringAgent* m_TargetAgent{ nullptr };

	virtual void PredictAndSetTarget(const float predictedTime);
	
};

// SteeringBehaviors...
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() override = default;

	// Seek Behavior
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;

protected:
};

class Flee : public ISteeringBehavior
{
public:
	Flee() = default;
	virtual ~Flee() override = default;

	// Flee Behavior
	virtual SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& Agent) override;

protected:
};

class Arrive final : public Seek
{
public:
	Arrive() = default;
	virtual ~Arrive() override = default;

	// Arrive Behavior
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

	// Face Behavior
	virtual SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& Agent) override;
protected:
};

class Pursuit final : public Seek
{
public:
	Pursuit() = default;
	virtual ~Pursuit() override = default;

	// Pursuit Behaviour
	virtual SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& Agent) override;

private:
	const float m_PredictionTimer{ 0.2f };
};

class Evade final : public Flee
{
public:
	Evade() = default;
	virtual ~Evade() override = default;

	// Evade Behaviour
	virtual SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& Agent) override;

private:
	const float m_PredictionTimer{ 0.2f };
	const float m_EvadeRadius{ 150.f };
	
	bool IsActorInTargetRange(ASteeringAgent& Agent, const FVector2D& CircleCenter) const;
};

class Wander final : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() override = default;

	// Wander Logic
	virtual SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& Agent) override;

protected:
private:
	const float m_OffsetDistance{ 200.f };
	const float m_WanderRadius{ 120.f };

	const float m_MaxAngleChange{ FMath::DegreesToRadians(60.f) };

	float m_ChangeTimer{ 0.f };
	const float m_MaxTargetChangeInterval{ 0.3f };

	float m_LastOnSwitchAngle{};

	FVector2D GetRandomPointInCircle(const FVector2D& circleCenter);
};

