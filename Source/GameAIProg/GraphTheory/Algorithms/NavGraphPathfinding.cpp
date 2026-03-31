#include "NavGraphPathfinding.h"

#include "AStar.h"
#include "PathSmoothing.h"
#include "VectorTypes.h"
#include "Shared/Graph/NavGraph/NavGraph.h"
#include "Shared/Graph/NavGraph/NavGraphNode.h"

//#include "Shared/Utils/GeoUtilities.h"

using namespace GameAI;

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos,
	NavGraph* const pNavGraph, std::vector<FVector2D>& debugNodePositions, std::vector<NavLine>& debugPortals) 
{
	//Create the path to return
	std::vector<FVector2D> FinalPath{};

	const auto NavTriPolygon{ pNavGraph->GetNavPolygon() };
	
	// ----- 1. Get Valid Start/End triangles -----
	// Get the start and endTriangle
	const TriPolygon::Triangle* StartTriangle{ NavTriPolygon->GetTriangleAtPosition( startPos, false ) };
	const TriPolygon::Triangle* EndTriangle{ NavTriPolygon->GetTriangleAtPosition( endPos, false ) };
	
	// Check if we have valid start/end triangles and they are not the same
	if (!StartTriangle || !EndTriangle || StartTriangle == EndTriangle)
	{
		return FinalPath;
	}
	
	// ------ 2. Start looking for a path ------
	
	// Copy the graph
	const auto ClonedGraph = pNavGraph->Clone();

	// * Create Extra node for the Start Node (Agent's position
	auto StartNode = std::make_unique<NavGraphNode>( startPos, -1 );
	const int StartNodeId{ ClonedGraph->AddNode( std::move(StartNode) ) };
	
	for (const auto& TriEdge : StartTriangle->GetEdges())
	{
		const int EdgeIdx{ NavTriPolygon->FindEdgeIndex(TriEdge).value_or(-1) };
		const int NodeId{ ClonedGraph->GetNodeIdFromEdgeIndex(EdgeIdx) };
		
		if (NodeId != Graphs::InvalidNodeId)
		{
			const float Cost{static_cast<float>( FVector2D::Distance( startPos, 
				ClonedGraph->GetNode( NodeId )->GetPosition() )) };
			
			ClonedGraph->AddConnection( StartNodeId, NodeId );
			ClonedGraph->FindConnection( StartNodeId, NodeId )->SetWeight( Cost );
		}
	}

	// * Create extra node for the endNode
	auto EndNode = std::make_unique<NavGraphNode>( endPos, -1 );
	const int EndNodeId{ ClonedGraph->AddNode( std::move(EndNode) ) };
	
	// Connect end node to all nodes on the edges of the end triangle
	for (const auto& TriEdge : EndTriangle->GetEdges())
	{
		const int EdgeIdx = NavTriPolygon->FindEdgeIndex(TriEdge).value_or(-1);
		const int NodeId = ClonedGraph->GetNodeIdFromEdgeIndex(EdgeIdx);
    
		if (NodeId != Graphs::InvalidNodeId)
		{
			const float Cost{static_cast<float>( FVector2D::Distance( endPos, 
				ClonedGraph->GetNode( NodeId )->GetPosition() )) };
			
			ClonedGraph->AddConnection( NodeId, EndNodeId );
			ClonedGraph->FindConnection( NodeId, EndNodeId )->SetWeight( Cost );
		}
	}
	
	// ------- 3. Run A star on new graph -------
	const AStar Pathfinder { AStar(ClonedGraph.get(), HeuristicFunctions::Euclidean) };
	
	// Need to find by id since Start and End Node are moved
	const std::vector<Node*> PathNodes{ Pathfinder.FindPath( 
		ClonedGraph->GetNode(StartNodeId).get(), 
		 ClonedGraph->GetNode(EndNodeId).get() ) };
	
	for (const Node* FinalNode : PathNodes)
	{
		FinalPath.push_back( FinalNode->GetPosition() );
	}
	
	//Debug Visualisation
	
	// Extra: Run optimiser on new graph (First check if everything works without SSFA!)
	for (const auto& pNode : ClonedGraph->GetNodes())
	{
		debugNodePositions.push_back(pNode->GetPosition());
	}
	
	if (!PathNodes.empty())
	{
		debugPortals = SSFA::FindPortals(PathNodes, *pNavGraph->GetNavPolygon());
		const std::vector<FVector2D> SmoothedPath = 
			SSFA::OptimizePortals(debugPortals, *pNavGraph->GetNavPolygon());
		
		if (!SmoothedPath.empty())
			FinalPath = SmoothedPath;
	}
	
	return FinalPath;
}

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos, NavGraph* const pNavGraph)
{
	std::vector<FVector2D> debugNodePositions{};
	std::vector<NavLine> debugPortals{};

	return FindPath(startPos, endPos, pNavGraph, debugNodePositions, debugPortals);
}