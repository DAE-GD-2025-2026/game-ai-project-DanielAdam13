// Fill out your copyright notice in the Description page of Project Settings.

#include "SteeringAgent.h"
#include "AIController.h"


// Sets default values
ASteeringAgent::ASteeringAgent()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASteeringAgent::BeginPlay()
{
	Super::BeginPlay();
}

void ASteeringAgent::BeginDestroy()
{
	Super::BeginDestroy();
}

// Called every frame
void ASteeringAgent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, "Calc");

	if (SteeringBehavior)
	{
		SteeringOutput Output { SteeringBehavior->CalculateSteering(DeltaTime, *this)};
		
		if (Output.IsValid)
		{
			AddMovementInput(FVector{ Output.LinearVelocity, 0.f });
			
			if (!IsAutoOrienting())
			{
				if (AAIController* AaiController = Cast<AAIController>(GetController()))
				{
					const float DeltaYaw{FMath::Clamp(Output.AngularVelocity, -1.f, 1.f) * 
						GetMaxAngularSpeed() * DeltaTime};
					
					const FRotator CurrentRotation{GetActorForwardVector().ToOrientationRotator()};
					const FRotator DeltaRotation{0, DeltaYaw, 0};
					const FRotator DesiredRotation{CurrentRotation + DeltaRotation};
					
					if (FMath::IsNearlyEqual(CurrentRotation.Yaw, DesiredRotation.Yaw))
						return;
					
					AaiController->SetControlRotation(DesiredRotation);
					FaceRotation(DesiredRotation);
					
				}
			}
		}
	}
}

// Called to bind functionality to input
void ASteeringAgent::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ASteeringAgent::SetSteeringBehavior(ISteeringBehavior* NewSteeringBehavior)
{
	SteeringBehavior = NewSteeringBehavior;
}