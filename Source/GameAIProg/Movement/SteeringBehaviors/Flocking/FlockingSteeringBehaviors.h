#pragma once
#include "Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
class Flock;

//COHESION - FLOCKING
//*******************
class Cohesion final : public Seek
{
public:
	Cohesion(Flock* const pFlock) :pFlock(pFlock) {};

	//Cohesion Behavior
	virtual SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& pAgent) override;

private:
	Flock* pFlock = nullptr;
};

//SEPARATION - FLOCKING
//*********************
class Separation final : public Seek
{
public:
	Separation(Flock* const pFlock) :pFlock(pFlock) {};

	//Separation Behavior
	virtual SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& pAgent) override;

private:
	Flock* pFlock = nullptr;
};

//VELOCITY MATCH - FLOCKING
//************************
class VelocityMatch final : public Seek
{
public:
	VelocityMatch(Flock* const pFlock) :pFlock(pFlock) {};

	//VelocityMatch Behavior
	virtual SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& pAgent) override;

private:
	Flock* pFlock = nullptr;
};
