#include "CombinedSteeringBehaviors.h"
#include <algorithm>
#include "../SteeringAgent.h"

#include <random>

BlendedSteering::BlendedSteering(const std::vector<WeightedBehavior>& WeightedBehaviors)
	:WeightedBehaviors(WeightedBehaviors)
{
};

//****************
//BLENDED STEERING
SteeringOutput BlendedSteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput BlendedSteering = {};
	// TODO: Calculate the weighted average steeringbehavior
	
	float AverageWeight{};
	for (const WeightedBehavior& wb : WeightedBehaviors)
	{
		AverageWeight += wb.Weight;
		wb.pBehavior->CalculateSteering(DeltaT, Agent);
	}
	AverageWeight /= WeightedBehaviors.size(); // 2 - only seek and wander
	
	for (const WeightedBehavior& wb : WeightedBehaviors)
	{
		BlendedSteering.LinearVelocity +=
			wb.pBehavior->CalculateSteering(DeltaT, Agent).LinearVelocity * AverageWeight;
	}
	
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, "Calc");

	return BlendedSteering;
}

float* BlendedSteering::GetWeight(ISteeringBehavior* const SteeringBehavior)
{
	// Find first weighted behavior with matching SteeringBehavior
	auto it = find_if(WeightedBehaviors.begin(),
		WeightedBehaviors.end(),
		[SteeringBehavior](const WeightedBehavior& Elem)
		{
			return Elem.pBehavior == SteeringBehavior;
		}
	);

	// return its weight
	if(it!= WeightedBehaviors.end())
		return &it->Weight;
	
	return nullptr;
}

//*****************
//PRIORITY STEERING
SteeringOutput PrioritySteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering = {};

	for (ISteeringBehavior* const pBehavior : m_PriorityBehaviors)
	{
		Steering = pBehavior->CalculateSteering(DeltaT, Agent);

		if (Steering.IsValid)
			break;
	}

	//If non of the behavior return a valid output, last behavior is returned
	return Steering;
}