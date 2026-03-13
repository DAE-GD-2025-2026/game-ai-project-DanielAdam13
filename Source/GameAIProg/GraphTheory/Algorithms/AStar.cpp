#include "AStar.h"

#include <list>

using namespace GameAI;

AStar::AStar(Graph* const pGraph, HeuristicFunctions::Heuristic hFunction)
	: pGraph(pGraph)
	, HeuristicFunction(hFunction)
{
}

std::vector<Node*>AStar::FindPath(Node* const pStartNode, Node* const pGoalNode) const
{
	// --------------------------
	// Path, Open and Closed list initialization with the starting ESTIMATED COST
	// --------------------------
	std::vector<Node*> Path{};
	std::list<NodeRecord> OpenList{};
	std::list<NodeRecord> ClosedList{};
	NodeRecord CurrentNodeRecord{};
	
	NodeRecord StartRecord;
	StartRecord.pNode = pStartNode;
	StartRecord.pConnection = nullptr; // Start connection
	StartRecord.costSoFar = 0.f; // Starting cost
	StartRecord.estimatedTotalCost = GetHeuristicCost( pStartNode, pGoalNode ); // Worst-case scenario cost
	
	OpenList.push_back( StartRecord );
	
	// --------------------------
	// WHILE LOOP - Populate Open and Closed lists
	// --------------------------
	while (!OpenList.empty())
	{
		// 1. Get node with the lowest ESTIMATED cost
		CurrentNodeRecord = *std::min_element(OpenList.begin(), OpenList.end());
		
		if (CurrentNodeRecord.pNode == pGoalNode)
			break; // Exit while loop
		
		// 2. Get all connections from the node
		auto Connections{pGraph->FindConnectionsFrom(CurrentNodeRecord.pNode->GetId())};
		
		for (Connection* pConn : Connections)
		{
			Node* pNextNode{(pGraph->GetNode( pConn->GetToId()).get() )};
			
			// Worst-case scenario cost
			float CostSoFarNext{ CurrentNodeRecord.costSoFar + pConn->GetWeight() };
			
			// 3. Check closed list
			auto ClosedIterator{std::find_if( ClosedList.begin(), ClosedList.end(), 
				[pNextNode] (const NodeRecord& r) {return r.pNode == pNextNode; })};
			
			if (ClosedIterator != ClosedList.end())
			{
				if (ClosedIterator->costSoFar <= CostSoFarNext)
					continue; // Continue to next connection
				
				OpenList.erase( ClosedIterator ); // Remove the new closed
			}
			
			// 4. Check if connection leads to node already in OPEN LIST
			auto OpenIterator{std::find_if( OpenList.begin(), OpenList.end(), 
				[pNextNode](const NodeRecord& r) {return r.pNode == pNextNode; })};
			
			if (OpenIterator != OpenList.end())
			{
				if (OpenIterator->costSoFar <= CostSoFarNext) 
					continue;  // Continue to next connection
				OpenList.erase( OpenIterator ); // Remove the new open
			}
			
			// 5. Add New Record to OPEN LIST
			NodeRecord NewRecord{};
			NewRecord.pNode = pNextNode;
			NewRecord.pConnection = pConn;
			NewRecord.costSoFar = CostSoFarNext;
			NewRecord.estimatedTotalCost = CostSoFarNext + 
				GetHeuristicCost( pNextNode, pGoalNode ); // Progress for next connection
			
			OpenList.push_back( NewRecord );
		}
		
		OpenList.remove( CurrentNodeRecord ); // Remove from nodes to be checked
		ClosedList.push_back( CurrentNodeRecord ); // Locked in Record for Backtracking...
	}
	
	// --------------------------
	// BACKTRACKING - Create Path
	// --------------------------
	if (CurrentNodeRecord.pNode == pGoalNode) // Open and Closed list are ALREADY correct 
	{
		while (CurrentNodeRecord.pNode != pStartNode)
		{
			Path.push_back( CurrentNodeRecord.pNode );
			
			// Find the connection where the NODE is FROM
			int	FromID{ CurrentNodeRecord.pConnection->GetFromId() };
			auto it{std::find_if( ClosedList.begin(), ClosedList.end(),
				[FromID](const NodeRecord& r) { return r.pNode->GetId() == FromID; })};
			
			CurrentNodeRecord = *it;
		}
		
		Path.push_back( pStartNode );
		std::reverse( Path.begin(), Path.end() );
	}
	
	return Path;
}

float AStar::GetHeuristicCost(Node* const pStartNode, Node* const pEndNode) const
{
	FVector2D toDestination = pGraph->GetNode(pEndNode->GetId())->GetPosition() - 
		pGraph->GetNode(pStartNode->GetId())->GetPosition();
	
	return HeuristicFunction(abs(toDestination.X), abs(toDestination.Y));
}