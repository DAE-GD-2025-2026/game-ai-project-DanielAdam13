#include "Level_CombinedSteering.h"

#include "imgui.h"

// Sets default values
ALevel_CombinedSteering::ALevel_CombinedSteering()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_CombinedSteering::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize blended templated steering
	SeekBlendedTemplate = std::make_unique<Seek>();
	WanderBlendedTemplate = std::make_unique<Wander>();
	pTemplateBlendedSteering = std::make_unique<BlendedSteering>(
		std::vector<BlendedSteering::WeightedBehavior>{{SeekBlendedTemplate.get(), 0.f}, {WanderBlendedTemplate.get(), 0.f}});
	
	// Initialize priority templated steering
	EvadePriorityTemplate = std::make_unique<Evade>();
	WanderPriorityTemplate = std::make_unique<Wander>();
	pTemplatePrioritySteering = std::make_unique<PrioritySteering>(
		std::vector<ISteeringBehavior*>{EvadePriorityTemplate.get(), WanderPriorityTemplate.get()});
	
	
	// Initialize agents vector with 10 elements
	for (int i{}; i < 10; ++i)
	{
		//AddAgent(EBehaviors::Blended);
	}
	
	AddAgent( EBehaviors::Wanderer );
	for (int i{}; i < 1; ++i)
	{
		AddAgent(EBehaviors::Priority);
	}
}

void ALevel_CombinedSteering::AddAgent(EBehaviors behaviorType)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, "Added Agent");
	
	// Spawn at random positions
	const float TrimWorldSize{ TrimWorld->GetTrimWorldSize()};
	const double PosRandX{static_cast<double>(FMath::FRandRange(-TrimWorldSize, TrimWorldSize))};
	const double PosRandY{static_cast<double>(FMath::FRandRange(-TrimWorldSize, TrimWorldSize))};
	
	ASteeringAgent* NewAgent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, 
		FVector{PosRandX, PosRandY, 90},FRotator::ZeroRotator);

	ISteeringBehavior* NewBeh;
	switch (behaviorType)
	{
	case EBehaviors::Blended:
		NewBeh = pTemplateBlendedSteering.get();
		break;
	case EBehaviors::Priority:
		NewBeh = pTemplatePrioritySteering.get();
		if (WandererAgent)
		{
			NewBeh->SetAgentTarget( WandererAgent->Agent );
		}
		break;
	case EBehaviors::Wanderer:
		NewBeh = new Wander();
		break;
	default:
		throw std::exception("No such Behavior!");
	}
	
	if (behaviorType != EBehaviors::Wanderer)
	{
		NewAgent->SetSteeringBehavior(NewBeh);
		auto NewCombined = std::make_unique<FCombinedAgent>(NewAgent, NewBeh);
		CombinedAgents.push_back(std::move(NewCombined));
	}
	else
	{
		NewAgent->SetSteeringBehavior(NewBeh);
		WandererAgent =  std::make_unique<FCombinedAgent>(NewAgent, NewBeh);
	}
}

void ALevel_CombinedSteering::BeginDestroy()
{
	Super::BeginDestroy();
}



// Called every frame
void ALevel_CombinedSteering::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	static bool bSpawn{true};
	if (bSpawn)
	{
		//AddAgent();
		bSpawn = false;
	}
	
#pragma region UI
	//UI
	{
		//Setup
		bool windowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Game AI", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	
		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scroll wheel: zoom cam.");
		ImGui::Unindent();
	
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();
	
		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();
	
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();
	
		ImGui::Text("Flocking");
		ImGui::Spacing();
		ImGui::Spacing();
	
		if (ImGui::Checkbox("Debug Rendering", &CanDebugRender))
		{
   // TODO: Handle the debug rendering of your agents here :)
		}
		ImGui::Checkbox("Trim World", &TrimWorld->bShouldTrimWorld);
		if (TrimWorld->bShouldTrimWorld)
		{
			ImGuiHelpers::ImGuiSliderFloatWithSetter("Trim Size",
				TrimWorld->GetTrimWorldSize(), 1000.f, 3000.f,
				[this](float InVal) { TrimWorld->SetTrimWorldSize(InVal); });
		}
		
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
	
		ImGui::Text("Behavior Weights");
		ImGui::Spacing();


		ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",
			pTemplateBlendedSteering->GetWeightedBehaviorsRef()[0].Weight, 0.f, 1.f,
			[this](float InVal)
			{
				pTemplateBlendedSteering->GetWeightedBehaviorsRef()[0].Weight = InVal;
			}, "%.2f");
		
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander",
		pTemplateBlendedSteering->GetWeightedBehaviorsRef()[1].Weight, 0.f, 1.f,
		[this](float InVal)
		{
			pTemplateBlendedSteering->GetWeightedBehaviorsRef()[1].Weight = InVal;
		}, "%.2f");
	
		//End
		ImGui::End();
	}
#pragma endregion
	
	// --- Combined Steering Update ---
	// Implement handling mouse click input for seek
	for (auto& a : CombinedAgents)
	{
		UpdateTargetToMouse(a.get());
	}
	
 // TODO: implement Make sure to also evade the wanderer
}

void ALevel_CombinedSteering::UpdateTargetToMouse(const FCombinedAgent* Agent) const
{
	static FVector2d PreviousMouseTargetPos{ MouseTarget.Position };
	
	if (MouseTarget.Position != PreviousMouseTargetPos)
		Agent->Behavior->SetTarget(MouseTarget);
	
	PreviousMouseTargetPos = MouseTarget.Position;
	
}