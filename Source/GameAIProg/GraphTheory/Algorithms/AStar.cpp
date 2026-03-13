#include "AStar.h"

#include <list>

using namespace GameAI;

AStar::AStar(Graph* const pGraph, HeuristicFunctions::Heuristic hFunction)
	: pGraph(pGraph)
	, HeuristicFunction(hFunction)
{
}

std::vector<Node*>AStar::FindPath(Node* const pStartNode, Node* const pGoalNode)
{
	std::vector<Node*> path{};
	std::list<NodeRecord> OpenList{};
	std::list<NodeRecord> ClosedList{};
	NodeRecord CurrentNodeRecord{};
	
	NodeRecord StartRecord;
	StartRecord.pNode = pStartNode;
	StartRecord.pConnection = nullptr;
	StartRecord.costSoFar = 0.f;
	StartRecord.estimatedTotalCost = GetHeuristicCost( pStartNode, pGoalNode );
	
	OpenList.push_back( StartRecord );
	
	// WHILE LOOP
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
			
			// Calculate next cost
			float CostSoFarNext{CurrentNodeRecord.costSoFar + pConn->GetWeight()};
			
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
				GetHeuristicCost( pNextNode, pGoalNode ); // Progress for next conenction
			
			OpenList.push_back( NewRecord );
		}
		
		OpenList.remove( CurrentNodeRecord ); // Remove from nodes to be checked
		ClosedList.push_back( CurrentNodeRecord ); // Locked in Record for Backtracking...
	}
	return path;
}

float AStar::GetHeuristicCost(Node* const pStartNode, Node* const pEndNode) const
{
	FVector2D toDestination = pGraph->GetNode(pEndNode->GetId())->GetPosition() - 
		pGraph->GetNode(pStartNode->GetId())->GetPosition();
	
	return HeuristicFunction(abs(toDestination.X), abs(toDestination.Y));
}