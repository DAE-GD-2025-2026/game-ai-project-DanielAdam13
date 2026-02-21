// Fill out your copyright notice in the Description page of Project Settings.

#include "SteeringAgent.h"


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
	
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, "Calc");

	if (SteeringBehavior)
	{
		SteeringOutput output = SteeringBehavior->CalculateSteering(DeltaTime, *this);

		if (!FVector{ output.LinearVelocity, 0.f }.IsNearlyZero())
		{
			AddMovementInput(FVector{ output.LinearVelocity, 0.f });
		}

		if (!FMath::IsNearlyZero(output.AngularVelocity))
		{
			FRotator newRotation = GetActorRotation();
			newRotation.Yaw += static_cast<double>(output.AngularVelocity) * DeltaTime;
			SetActorRotation(newRotation);
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