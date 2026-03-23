#include "NavGraph.h"

#include "NavGraphNode.h"

GameAI::NavGraph::NavGraph(std::unique_ptr<TriPolygon> && NavPoly)
	: Graph{false}
	, pNavPoly{std::move(NavPoly)}
{
	CreateNavigationGraph();
}

GameAI::NavGraph::NavGraph(const NavGraph& Other)
	: Graph(false)
{
	Nodes.reserve(Other.Nodes.size());
	for (std::unique_ptr<Node> const & OtherNode : Other.Nodes)
	{
		Nodes.push_back(std::make_unique<NavGraphNode>(*dynamic_cast<NavGraphNode*>(OtherNode.get())));
	}
        
	Connections.reserve(Other.Connections.size());
	for (std::unique_ptr<Connection> const & OtherConnection : Other.Connections)
	{
		Connections.push_back(std::make_unique<Connection>(*OtherConnection.get()));
	}
}

std::unique_ptr<GameAI::NavGraph> GameAI::NavGraph::Clone() const
{
	return std::make_unique<NavGraph>(*this);
}

int GameAI::NavGraph::GetNodeIdFromEdgeIndex(int EdgeIdx) const
{
	if (EdgeIdx >= 0)
	{
		for (auto const & pNode : Nodes)
		{
			if (reinterpret_cast<NavGraphNode*>(pNode.get())->GetEdgeIdx() == EdgeIdx)
			{
				return pNode->GetId();
			}
		}
	}
	
	return Graphs::InvalidNodeId;
}

void GameAI::NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigation mesh and create nodes
	for (const auto& Edge : pNavPoly->GetEdges())
	{
		const FVector& EdgeMidPoint{
			(Edge.GetP1( *pNavPoly.get() ) + Edge.GetP2( *pNavPoly.get() )) / 2};
		
		this->AddNode( std::make_unique<Node>( FVector2D(EdgeMidPoint.X, EdgeMidPoint.Y)) );
	}

	//2. Create connections now that every node is created	
		//2 valid nodes -> 1 connection
		//3 valid nodes -> 3 connections
	for (const auto& Triangle : pNavPoly->GetTriangles())
	{
		std::vector<int> TriangleNodeIndices{};
		
		// Find and store all node indices
		for (const auto& TriEdge : Triangle.GetEdges())
		{
			const int EdgeIdx{pNavPoly->FindEdgeIndex( TriEdge ).value_or( -1 )};
			const int NodeIdx{GetNodeIdFromEdgeIndex( EdgeIdx )};
			
			if (NodeIdx != Graphs::InvalidNodeId)
			{
				TriangleNodeIndices.push_back( NodeIdx );
			}
		}
		
		// Then Make connections - 3 for a triangle
		for (size_t i{}; i < TriangleNodeIndices.size(); ++i)
		{
			for (size_t j{ i + 1 }; j < TriangleNodeIndices.size(); ++j)
			{
				this->AddConnection( std::make_unique<Connection>( TriangleNodeIndices[i], 
					TriangleNodeIndices[j]) );
			}
		}
	}

	//3. Set the connections cost to the actual distance
	for (const auto& Connection : this->GetConnections())
	{
		const FVector2D& PosA{ GetNode(Connection->GetFromId())->GetPosition() };
		const FVector2D& PosB{ GetNode(Connection->GetToId())->GetPosition() };
		
		Connection->SetWeight( FVector2D::Distance( PosA, PosB ) );
	}
}
