#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

#include "DrawDebugHelpers.h"

SteeringOutput ISteeringBehavior::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    if (Agent.GetWorld())
    {
        // --- Draw target point ---
        // DrawDebugSphere(
        //     Agent.GetWorld(),
        //     FVector(Target.Position, Agent.GetActorLocation().Z),
        //     10.f,
        //     12,
        //     FColor::Red
        // );
    }

    return SteeringOutput();
}

void ISteeringBehavior::PredictAndSetTarget(const float predictedTime)
{
    FTargetData newTarget;
    newTarget.Position = m_TargetAgent->GetPosition() + m_TargetAgent->GetLinearVelocity() * predictedTime;

    SetTarget(newTarget);
}

SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
    ISteeringBehavior::CalculateSteering(DeltaT, Agent);
    SteeringOutput Steering{};

    Steering.LinearVelocity = Target.Position - Agent.GetPosition();

    currentLinearVelocity = Steering.LinearVelocity;
    
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

    FVector2D toTarget{ Target.Position - Agent.GetPosition() };
    if (toTarget.IsNearlyZero())
        return SteeringOutput();
    toTarget.Normalize();

    FVector2D agentForward{ Agent.GetActorForwardVector() };
    agentForward.Normalize();

    float dot{ static_cast<float>(FVector2D::DotProduct(agentForward, toTarget)) };
    dot = FMath::Clamp(dot, -1.f, 1.f);

    // Get desired angle
    const float angle{ FMath::Acos(dot) };
    const float sign{ static_cast<float>(FMath::Sign(agentForward.X * toTarget.Y - agentForward.Y * toTarget.X)) };
    const float signedAngle{ angle * sign }; // radians

    Steering.AngularVelocity = FMath::RadiansToDegrees(signedAngle);

    return Steering;
}

SteeringOutput Wander::CalculateSteering(float deltaT, ASteeringAgent& Agent)
{
    SteeringOutput Steering{};

    const FVector2D CircleCenter{ Agent.GetActorLocation() + Agent.GetActorForwardVector() * m_OffsetDistance };
    
    m_ChangeTimer += deltaT;

    // Change Target
    if (m_ChangeTimer >= m_MaxTargetChangeInterval)
    {
        FTargetData newTarget{};
        newTarget.Position = GetRandomPointInCircle(CircleCenter);
        //GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::SanitizeFloat(newTarget.Position.X) + ", " + FString::SanitizeFloat(newTarget.Position.Y));
        ISteeringBehavior::SetTarget(newTarget);
        //GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::SanitizeFloat(Target.Position.X) + ", " + FString::SanitizeFloat(Target.Position.Y));

        m_ChangeTimer -= m_MaxTargetChangeInterval;
    }

    // ----- Draw the wander circle -----
    // if (Agent.GetWorld())
    // {
    //     DrawDebugCircle(
    //         Agent.GetWorld(),
    //         FVector(CircleCenter.X, CircleCenter.Y, Agent.GetActorLocation().Z), // center
    //         m_WanderRadius,  
    //         32, 
    //         FColor::Blue, 
    //         false, 
    //         -1.f, 
    //         0,   
    //         2.f,  
    //         FVector(1, 0, 0), 
    //         FVector(0, 1, 0), 
    //         true 
    //     );
    // }

    // Always seek the target
    Steering = Seek::CalculateSteering(deltaT, Agent);

    return Steering;
}

FVector2D Wander::GetRandomPointInCircle(const FVector2D& circleCenter)
{
    // Generate random angle dependent on the last chosen angle and in the range of the max angle change
    const float angle{ m_LastOnSwitchAngle + FMath::FRandRange(-m_MaxAngleChange, m_MaxAngleChange) };

    const FVector2D offset{ FVector2D(FMath::Cos(angle), FMath::Sin(angle)) * m_WanderRadius };
    const FVector2D wanderTargetPos{ circleCenter + offset };

    m_LastOnSwitchAngle = angle;

    return wanderTargetPos;
}

SteeringOutput Pursuit::CalculateSteering(float deltaT, ASteeringAgent& Agent)
{
    SteeringOutput Steering{};

    // If target doesn't exist -> early out
    if (!m_TargetAgent)
        return Steering;

    PredictAndSetTarget(m_PredictionTimer);

    // Agent has updated target
    Steering = Seek::CalculateSteering(deltaT, Agent);

    // Seek to the new predicted target
    return Steering;
}

SteeringOutput Evade::CalculateSteering(float deltaT, ASteeringAgent& Agent)
{
    SteeringOutput Steering{};

    // If target doesn't exist -> early out
    if (!m_TargetAgent)
        return Steering;

    PredictAndSetTarget(m_PredictionTimer);

    // Agent has updated target
    Steering = Flee::CalculateSteering(deltaT, Agent);
    
    // Draw evade circle around target
    if (Agent.GetWorld())
    {
        DrawDebugCircle(
            Agent.GetWorld(),
            FVector(Target.Position.X, Target.Position.Y, Agent.GetActorLocation().Z), // center
            m_EvadeRadius,  
            32, 
            FColor::Purple, 
            false, 
            -1.f, 
            0,   
            2.f,  
            FVector(1, 0, 0), 
            FVector(0, 1, 0), 
            true 
        );
    }
    
    // Change the IsValid flag dependent on if evade actor is inside of target range
    // The flag is used in Priority Steering
    Steering.IsValid = !IsActorInTargetRange(Agent, Target.Position);

    // Seek to the new predicted target
    return Steering;
}

bool Evade::IsActorInTargetRange(ASteeringAgent& Agent, const FVector2D& CircleCenter) const
{
    const FVector2D AgentLocation{ Agent.GetActorLocation()};
    
    return FVector2D::DistSquared(AgentLocation, CircleCenter) <= m_EvadeRadius * m_EvadeRadius;
}
