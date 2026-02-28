// Fill out your copyright notice in the Description page of Project Settings.


#include "Level_Flocking.h"


// Sets default values
ALevel_Flocking::ALevel_Flocking()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_Flocking::BeginPlay()
{
	Super::BeginPlay();

	TrimWorld->SetTrimWorldSize(3000.f);
	TrimWorld->bShouldTrimWorld = true;
	
	pAgentToEvade = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, 
		FVector{0, 0, 90},FRotator::ZeroRotator);
	AgentToEvadeSteering = std::make_unique<Arrive>(  );
	AgentToEvadeSteering->SetTarget( MouseTarget );
	pAgentToEvade->SetSteeringBehavior(AgentToEvadeSteering.get());

	pFlock = std::make_unique<Flock>(GetWorld(), SteeringAgentClass, FlockSize,
			TrimWorld->GetTrimWorldSize(), pAgentToEvade,true);
}

// Called every frame
void ALevel_Flocking::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	AgentToEvadeSteering->SetTarget( MouseTarget );

	pFlock->ImGuiRender(WindowPos, WindowSize);
	pFlock->Tick(DeltaTime);
	pFlock->RenderDebug();
	if (bUseMouseTarget)
		pFlock->SetTarget_Seek(MouseTarget);
	
	
}

