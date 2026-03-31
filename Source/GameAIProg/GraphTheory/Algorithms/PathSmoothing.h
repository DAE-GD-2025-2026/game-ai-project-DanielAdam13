#pragma once
#include <vector>

#include "NavGraphPathfinding.h"
#include "Movement/Pathfinding/Navmesh/TriPolygon.h"
#include "Shared/Graph/Graph.h"
#include "Shared/Graph/NavGraph/NavGraphNode.h"

namespace GameAI
{
	class SSFA final
{
public:
	//=== SSFA Functions ===
	//--- References ---
	//http://digestingduck.blogspot.be/2010/03/simple-stupid-funnel-algorithm.html
	//https://gamedev.stackexchange.com/questions/68302/how-does-the-simple-stupid-funnel-algorithm-work
	static std::vector<NavLine> FindPortals(std::vector<Node*> const & Path, TriPolygon const & NavPoly)
	{
		//Container
		std::vector<NavLine> Portals = {};
		
		// 1. Get and add Start Pos to the Portals
		const FVector2D StartPos{ Path[0]->GetPosition() };
		Portals.push_back(NavLine(StartPos, StartPos));
		
		//For each node received, get it's corresponding line
		for (size_t i{ 1 }; i < Path.size() - 1; ++i)
		{
			const auto* NavNode{ static_cast<const NavGraphNode*>(Path[i]) };
			int EdgeIdx{ NavNode->GetEdgeIdx() };
			
			const auto& Edge{ NavPoly.GetEdges()[EdgeIdx] };
			
			FVector2D P1{ FVector2D(Edge.GetP1( NavPoly )) };
			FVector2D P2{ FVector2D(Edge.GetP2( NavPoly )) };
			
			// --- Determine direction ---
			const FVector2D PathPos{ Path[i - 1]->GetPosition() };
			const FVector2D Dir{ Path[i]->GetPosition() - PathPos };
	
			//Redetermine it's "orientation" based on the required path (left-right vs right-left) - 
			//p1 should be right point
			const float Cross{ static_cast<float>(Dir.X * (P1.Y - P2.Y) - Dir.Y * (P1.X - P2.X)) };
			
			if (Cross > 0.f)
				Swap( P1, P2 );
			
			//Store portal
			Portals.push_back( NavLine(P1, P2) );
		}
		
		//Add degenerate portal to force end evaluation
		const FVector2D EndPos{ Path.back()->GetPosition() };
		Portals.push_back(NavLine(EndPos, EndPos));

		return Portals;
	}

	static std::vector<FVector2D> OptimizePortals( std::vector<NavLine> const & Portals, TriPolygon const & NavPoly)
	{
		std::vector<FVector2D> Path{};
		
		FVector2D ApexPoint{ Portals[0].P1 };
		Path.push_back( ApexPoint );
		
		// Right and Left "legs" before the optimization:
		FVector2D RightLeg{ Portals[1].P1 - ApexPoint };
		FVector2D LeftLeg{ Portals[1].P2 - ApexPoint };
		size_t RightLegIndex{ 1 };
		size_t LeftLegIndex{ 1 };
		
		for (size_t i{ 1 }; i < Portals.size(); ++i)
		{
			//P1 == right point of portal, P2 == left point of portal
			NavLine CurrentPortal{ Portals[i] };
			
			//--- RIGHT CHECK ---
			const FVector2D NewRightLeg{ CurrentPortal.P1 - ApexPoint };

			//1. See if moving funnel inwards - RIGHT
			const float RightCross{ static_cast<float>(RightLeg.X * NewRightLeg.Y - RightLeg.Y * NewRightLeg.X) };
			
			// If inwards (CCW)
			if (RightCross <= 0.f)
			{
				const float LeftCross{ static_cast<float>(LeftLeg.X * NewRightLeg.Y - LeftLeg.Y * NewRightLeg.X) };
				
				//2. See if new line degenerates a line segment - RIGHT
				// If crossed over left leg
				if (LeftCross > 0.f)
				{
					// Leftleg becomes new apex point
					// Update Apex to left leg tip
					ApexPoint = Portals[LeftLegIndex].P2;
					
					// UPDATE PORTAL INDEX
					size_t ApexIndex{ LeftLegIndex };
					i = ApexIndex + 1;
					
					//Calculate new legs (if not the end)
					RightLegIndex = i; // reset both legs to new portal
					LeftLegIndex = i;  // reset both legs to new portal
					
					Path.push_back( ApexPoint );

					if (i < Portals.size())
					{
						RightLeg = Portals[RightLegIndex].P1 - ApexPoint;
						LeftLeg = Portals[LeftLegIndex].P2 - ApexPoint;
					}
					continue;
				}
				else
				{
					RightLeg = NewRightLeg;
					RightLegIndex = i;
				}
			}
			
			//--- LEFT CHECK ---
			//1. See if moving funnel inwards - LEFT
			const FVector2D NewLeftLeg{ CurrentPortal.P2 - ApexPoint };
			const float NewLeftCross{ static_cast<float>(LeftLeg.X * NewLeftLeg.Y - LeftLeg.Y * NewLeftLeg.X) };

			//2. See if new line degenerates a line segment - LEFT
			if (NewLeftCross >= 0.f) // Going inwards (CW)
			{
				const float RightCrossLeft{ static_cast<float>(RightLeg.X * NewLeftLeg.Y - RightLeg.Y * NewLeftLeg.X) };

				if (RightCrossLeft < 0.f) // Crossed over right leg -> new apex
				{
					// Rightleg becomes new apex point
					ApexPoint = Portals[RightLegIndex].P1;
					// Update the portal index
					i = RightLegIndex + 1;
					
					//Calculate new legs (if not the end)
					RightLegIndex = i;
					LeftLegIndex  = i;

					Path.push_back(ApexPoint);

					if (i < Portals.size())
					{
						RightLeg = Portals[i].P1 - ApexPoint;
						LeftLeg  = Portals[i].P2 - ApexPoint;
					}
					continue;
				}
				else // Tighten left
				{
					LeftLeg = NewLeftLeg;
					LeftLegIndex = i;
				}
			}
		}

		// Add last path point
		Path.push_back( Portals.back().P1 );
		
		return Path;
	}
private:
	SSFA() {};
	~SSFA() {};
};
}
