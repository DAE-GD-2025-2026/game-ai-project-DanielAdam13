#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Steering{};
	
	if (!pFlock)
		return Steering;
	if (pFlock->GetNrOfNeighbors() == 0)
		return Steering;
	
	Target.Position = pFlock->GetAverageNeighborPos();
	Steering = Seek::CalculateSteering(deltaT, pAgent);
	
	return Steering;
}
//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	float Force{ 0.f };
	
	for (ASteeringAgent* neigh : pFlock->GetNeighbors())
	{
		const FVector toAgent{pAgent.GetActorLocation() - neigh->GetActorLocation()};
		float Distance{static_cast<float>(toAgent.Length())};
	}
	
	return Seek::CalculateSteering( deltaT, pAgent );
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	return Seek::CalculateSteering( deltaT, pAgent );
}
