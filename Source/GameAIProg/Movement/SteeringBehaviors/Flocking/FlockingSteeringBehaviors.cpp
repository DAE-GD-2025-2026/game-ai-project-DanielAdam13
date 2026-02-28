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
	if (pFlock->GetNrOfNeighbors() <= 0)
		return Steering;
	
	Target.Position = pFlock->GetAverageNeighborPos();
	Steering = Seek::CalculateSteering(deltaT, pAgent);
	
	return Steering;
}
//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Steering{};
	FVector2D Force{ 0.f };
	
	const auto& Neighbors{pFlock->GetNeighbors()};
	
	for (int i = 0; i < pFlock->GetNrOfNeighbors(); ++i)
	{
		const FVector NeighborToAgent{pAgent.GetActorLocation() - 
			Neighbors[i]->GetActorLocation()};

		const FVector2D Offset(NeighborToAgent.X, NeighborToAgent.Y);
		float DistSqr = Offset.SizeSquared();

		if (DistSqr > 1.f)
			Force += Offset / DistSqr;
	}

	Force.Normalize();
	Force *= pAgent.GetMaxLinearSpeed();
	
	Steering.LinearVelocity = Force;
	Steering.IsValid = true;
	return Steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Steering{};
	
	const FVector2D AverageVelocity{pFlock->GetAverageNeighborVelocity()};
	Steering.LinearVelocity = AverageVelocity - pAgent.GetLinearVelocity();
	
	return Steering;
}
