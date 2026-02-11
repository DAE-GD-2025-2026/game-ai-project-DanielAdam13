#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

#include "DrawDebugHelpers.h"

//SEEK
//*******
// TODO: Do the Week01 assignment :^)

SteeringOutput ISteeringBehavior::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    // Draw target point
    DrawDebugSphere(
        GWorld,
        FVector(Target.Position, Agent.GetActorLocation().Z),
        10.f,
        12,
        FColor::Red
    );

    return SteeringOutput();
}

SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    ISteeringBehavior::CalculateSteering(DeltaT, Agent);
    SteeringOutput Steering{};

    Steering.LinearVelocity = Target.Position - Agent.GetPosition();

    return Steering;
}

SteeringOutput Flee::CalculateSteering(float deltaT, ASteeringAgent& Agent)
{
    ISteeringBehavior::CalculateSteering(deltaT, Agent);
    SteeringOutput Steering{};

    Steering.LinearVelocity = Agent.GetPosition() - Target.Position;

    return Steering;
}

SteeringOutput Arrive::CalculateSteering(float deltaT, ASteeringAgent& Agent)
{
    ISteeringBehavior::CalculateSteering(deltaT, Agent);
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
    ISteeringBehavior::CalculateSteering(deltaT, Agent);
    SteeringOutput Steering{};

    Steering.AngularVelocity = FMath::FindDeltaAngleDegrees(Agent.GetRotation(), Target.Orientation);

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
    ISteeringBehavior::CalculateSteering(deltaT, Agent);
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

    // Agent position and forward direction
    FVector AgentPos = Agent.GetActorLocation();
    FVector ForwardDir = Agent.GetActorForwardVector();

    FVector CircleCenter = AgentPos + ForwardDir * m_OffsetDistance;

    // Draw the circle
    if (GWorld)
    {
        DrawDebugCircle(
            GWorld,                     // world
            CircleCenter,               // center
            m_WanderRadius,             // radius
            32,                         // segments
            FColor::Blue,               // color
            false,                      // persistent
            -1.f,                       // lifetime
            0,                          // depth priority
            2.f,                        // thickness
            FVector(1, 0, 0),             // X axis
            FVector(0, 1, 0),             // Y axis
            true                        // draw axis (true = circle in XY plane)
        );

        // Optionally draw the agent’s forward vector
        DrawDebugLine(
            GWorld,
            AgentPos,
            CircleCenter,
            FColor::Red,
            false,
            -1.f,
            0,
            2.f
        );
    }

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
