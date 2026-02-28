#include "SpacePartitioning.h"

// --- Cell ---
// ------------
Cell::Cell(float Left, float Bottom, float Width, float Height)
{
	BoundingBox.Min = { Left, Bottom };
	BoundingBox.Max = { BoundingBox.Min.X + Width, BoundingBox.Min.Y + Height };
}

std::vector<FVector2D> Cell::GetRectPoints() const
{
	const float left = BoundingBox.Min.X;
	const float bottom = BoundingBox.Min.Y;
	const float width = BoundingBox.Max.X - BoundingBox.Min.X;
	const float height = BoundingBox.Max.Y - BoundingBox.Min.Y;

	std::vector<FVector2D> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(UWorld* pWorld, float Width, float Height, int Rows, int Cols, int MaxEntities)
	: pWorld{pWorld}
	, SpaceWidth{Width}
	, SpaceHeight{Height}
	, NrOfRows{Rows}
	, NrOfCols{Cols}
	, NrOfNeighbors{0}
{
	Neighbors.SetNum(MaxEntities);
	
	//calculate bounds of a cell
	CellWidth = Width / Cols;
	CellHeight = Height / Rows;

	// Create the cells
	CellOrigin = FVector2D(-Width * 0.5f, -Height * 0.5f);
	for (int r{0}; r < Rows; ++r)
	{
		for (int c{0}; c < Cols; ++c)
		{
			const float Left{CellOrigin.X + c * CellWidth};
			const float Bottom{CellOrigin.Y + r * CellHeight};
			
			Cells.emplace_back( Left, Bottom, CellWidth, CellHeight );
		}
	}
}

void CellSpace::AddAgent(ASteeringAgent& Agent)
{
	// Add the agent to the correct cell
	const FVector ActorLocation{Agent.GetActorLocation()};
	const int Index{PositionToIndex(FVector2D(ActorLocation.X, ActorLocation.Y) )};
	
	Cells[Index].Agents.push_back( &Agent );
}

void CellSpace::UpdateAgentCell(ASteeringAgent& Agent, const FVector2D& OldPos)
{
	// Check if the agent needs to be moved to another cell.
	const int OldCellIndex{PositionToIndex( OldPos )};
	
	// Use the calculated index for oldPos and currentPos for this
	const FVector AgentLocation{Agent.GetActorLocation()};
	const int NewCellIndex{PositionToIndex( FVector2D(AgentLocation.X, AgentLocation.Y) )};
	
	if (OldCellIndex == NewCellIndex)
		return;
	
	// Remove from old, add to new
	Cells[OldCellIndex].Agents.remove( &Agent );
	Cells[NewCellIndex].Agents.push_back( &Agent );
}

void CellSpace::RegisterNeighbors(ASteeringAgent& Agent, float QueryRadius)
{
	// TODO Register the neighbors for the provided agent
	// TODO Only check the cells that are within the radius of the neighborhood
}

void CellSpace::EmptyCells()
{
	for (Cell& c : Cells)
		c.Agents.clear();
}

void CellSpace::RenderCells() const
{
	// TODO Render the cells with the number of agents inside of it
}

int CellSpace::PositionToIndex(FVector2D const & Pos) const
{
	// Calculate the index of the cell based on the position
	int Col{static_cast<int>(( Pos.X - CellOrigin.X ) / CellWidth)};
	int Row{static_cast<int>(( Pos.Y - CellOrigin.Y ) / CellHeight)};
	
	Col = FMath::Clamp( Col, 0, NrOfCols - 1 );
	Row = FMath::Clamp( Row, 0, NrOfRows - 1 );
	
	return Row * NrOfCols + Col;
}

bool CellSpace::DoRectsOverlap(FRect const & RectA, FRect const & RectB)
{
	// Check if the rectangles are separated on either axis
	if (RectA.Max.X < RectB.Min.X || RectA.Min.X > RectB.Max.X) return false;
	if (RectA.Max.Y < RectB.Min.Y || RectA.Min.Y > RectB.Max.Y) return false;
    
	// If they are not separated, they must overlap
	return true;
}