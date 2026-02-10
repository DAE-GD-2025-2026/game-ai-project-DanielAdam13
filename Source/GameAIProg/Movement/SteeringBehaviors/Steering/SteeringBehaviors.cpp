#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

//SEEK
//*******
// TODO: Do the Week01 assignment :^)

SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    SteeringOutput Steering{};

    Steering.LinearVelocity = Target.Position - Agent.GetPosition();

    return Steering;
}

SteeringOutput Flee::CalculateSteering(float deltaT, ASteeringAgent& Agent)
{
    SteeringOutput Steering{};

    Steering.LinearVelocity = Agent.GetPosition() - Target.Position;

    return Steering;
}

Arrive::Arrive()
    : maxLinVelocity{},
    outerRadius{ 300.f },
    innerRadius{ 20.f }
{
}

SteeringOutput Arrive::CalculateSteering(float deltaT, ASteeringAgent& Agent)
{
    SteeringOutput Steering{};

    FVector2D agentToTargetVector{ Target.Position - Agent.GetPosition() };
    float agentToTargetVectorLength{ float(agentToTargetVector.Length()) };

    if (agentToTargetVectorLength > outerRadius)
    {
        maxLinVelocity = Seek::CalculateSteering(deltaT, Agent).LinearVelocity;
        //GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Outside outer"));
        Steering.LinearVelocity = maxLinVelocity;
    }
    else if (agentToTargetVectorLength > innerRadius)
    {
        //GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Inside outer"));
        Steering.LinearVelocity = agentToTargetVector / (outerRadius - innerRadius);
    }
    else
    {
        //GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Inside inner"));
    }

    return Steering;
}
