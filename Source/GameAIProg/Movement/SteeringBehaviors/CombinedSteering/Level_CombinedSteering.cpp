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
	
	// Initialize templated steering
	pTemplateSteering = std::make_unique<BlendedSteering>();
	const auto seekBeh = std::make_unique<Seek>();
	const auto wanderBeh = std::make_unique<Wander>();
	
	const BlendedSteering::WeightedBehavior templateSeek{std::move(seekBeh.get()), 0.f};
	const BlendedSteering::WeightedBehavior templateWan{std::move(wanderBeh.get()), 1.f};
	pTemplateSteering->AddBehaviour(std::move(templateSeek));
	pTemplateSteering->AddBehaviour(std::move(templateWan));
	
	// Initialize agents vector with 10 Seekers
	// for (int i{}; i < 10; ++i)
	// {
	// 	AddAgent(Behaviors::Seek);
	// }
	
	AddAgent();
	AddAgent();
}

void ALevel_CombinedSteering::BeginDestroy()
{
	Super::BeginDestroy();
}

void ALevel_CombinedSteering::AddAgent()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, "Added Agent");
	
	CombinedAgent newAgent{};
	newAgent.Agent =  GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, FVector{0,0,90}, 
			FRotator::ZeroRotator);
	
	newAgent.Behavior = std::make_unique<BlendedSteering>();
	newAgent.Behavior->AddBehaviour(pTemplateSteering->GetWeightedBehaviorsRef()[0]);
	newAgent.Behavior->AddBehaviour(pTemplateSteering->GetWeightedBehaviorsRef()[1]);
	
	CombinedAgents.push_back(std::move(newAgent));
}

// Called every frame
void ALevel_CombinedSteering::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	static bool spawn{true};
	if (spawn)
	{
		//AddAgent();
		spawn = false;
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
		ImGui::Text("Scrollwheel: zoom cam.");
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
			pTemplateSteering->GetWeightedBehaviorsRef()[0].Weight, 0.f, 1.f,
			[this](float InVal)
			{
				pTemplateSteering->GetWeightedBehaviorsRef()[0].Weight = InVal;
			}, "%.2f");
		
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander",
		pTemplateSteering->GetWeightedBehaviorsRef()[1].Weight, 0.f, 1.f,
		[this](float InVal)
		{
			pTemplateSteering->GetWeightedBehaviorsRef()[1].Weight = InVal;
		}, "%.2f");
	
		//End
		ImGui::End();
	}
#pragma endregion
	
	// Combined Steering Update
 // TODO: implement handling mouse click input for seek
	
	for (auto& a : CombinedAgents)
	{
		if (a.Agent)
		{
			UpdateTarget(a);
		}
	}
	
 // TODO: implement Make sure to also evade the wanderer
}


void ALevel_CombinedSteering::UpdateTarget(CombinedAgent& agent)
{
	//if (agent.Behavior->)
}