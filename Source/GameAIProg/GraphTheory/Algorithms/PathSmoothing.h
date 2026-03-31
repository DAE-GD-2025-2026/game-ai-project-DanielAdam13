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
		//P1 == right point of portal, P2 == left point of portal
		
			//--- RIGHT CHECK ---
			//1. See if moving funnel inwards - RIGHT
			
				//2. See if new line degenerates a line segment - RIGHT
				
					//Leftleg becomes new apex point

					//Calculate new legs (if not the end)


			//--- LEFT CHECK ---
			//1. See if moving funnel inwards - LEFT

				//2. See if new line degenerates a line segment - LEFT

					//Rightleg becomes new apex point

					//Calculate new legs (if not the end)


		// Add last path point

		return Path;
	}
private:
	SSFA() {};
	~SSFA() {};
};
}
