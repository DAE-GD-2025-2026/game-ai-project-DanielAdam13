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

SteeringOutput Arrive::CalculateSteering(float deltaT, ASteeringAgent& Agent)
{
    SteeringOutput Steering{};

    FVector2D agentToTargetVector{ Target.Position - Agent.GetPosition() };
    float agentToTargetVectorLength{ float(agentToTargetVector.Length()) };

    // Outside outer radius
    if (agentToTargetVectorLength > m_OuterRadius)
    {
        m_MaxLinearVelocity = Seek::CalculateSteering(deltaT, Agent).LinearVelocity;
        Steering.LinearVelocity = m_MaxLinearVelocity;
    }
    // Between radiuses - slow down
    else if (agentToTargetVectorLength > m_InnerRadius)
    {
        Steering.LinearVelocity = agentToTargetVector / (m_OuterRadius - m_InnerRadius);
    }
    // Else it is default initialized -> 0f

    return Steering;
}

SteeringOutput Face::CalculateSteering(float deltaT, ASteeringAgent& Agent)
{
    SteeringOutput Steering{};

    //Steering.AngularVelocity = Target.Orientation - Agent.GetRotation();

    return Steering;
}

Wander::Wander()
    : m_NewTarget{}
{
    m_NewTarget.Position = GetRandomPointInCircle();
    ISteeringBehavior::SetTarget(m_NewTarget);
}

SteeringOutput Wander::CalculateSteering(float deltaT, ASteeringAgent& Agent)
{
    SteeringOutput Steering{};

    static float changeTimer{ m_MaxTargetChangeInterval };
    
    changeTimer += deltaT;
    if (changeTimer >= m_MaxTargetChangeInterval)
    {
        m_NewTarget.Position = GetRandomPointInCircle();
        ISteeringBehavior::SetTarget(m_NewTarget);

        //GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, m_NewTarget.Position.ToString());
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Switched"));

        changeTimer -= m_MaxTargetChangeInterval;
    }

    Steering = Seek::CalculateSteering(deltaT, Agent);

    return Steering;
}

FVector2D Wander::GetRandomPointInCircle()
{
    const float angle{ FMath::FRandRange(m_LastOnSwitchAngle - m_MaxAngleChange, m_LastOnSwitchAngle + m_MaxAngleChange) };

    const double doubRadius{ double(m_WanderRadius) };

    const FVector2D newPoint{ m_OffsetDistance + doubRadius * FMath::Cos(angle), m_OffsetDistance + doubRadius * FMath::Sin(angle) };


    m_LastOnSwitchAngle = angle;

    return newPoint;
}
