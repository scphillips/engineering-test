#include "MatchingGameExercise.h"

#include <random>
#include <stdexcept>

Board MatchingGameExercise::beginGame(int width, int height)
{
	Board gameBoard(width, height);
	// Generate a randomized grid with no starting matches
	// This function assumes there are more Jewel types than Cartesian adjacencies,
	//  and can therefore assign values without running out of valid choices
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			BoardCellCollection matchedCells;
			do
			{
				matchedCells.clear();
				gameBoard.setJewel(x, y, (JewelKind)(rand() % Violet + 1));
				addMatchingCellsToCollection(BoardCell(x, y), gameBoard, matchedCells, BoardCellCollection());
			}
			while (matchedCells.size() >= NumberOfColorsToMatch);
		}
	}

	return gameBoard;
}

RankedMoves MatchingGameExercise::calculateMovesForBoard(const Board& board)
{
	int boardWidth = board.getWidth();
	int boardHeight = board.getHeight();
	Board workingBoard = board; // Make a working copy for calculating potential moves
	Move workingMove;

	// Valid moves ordered by calculated score in ascending order
	RankedMoves potentialMoves;

	// Traverse the board sequentially
	for (int y = 0; y < boardHeight; y++)
	{
		for (int x = 0; x < boardWidth; x++)
		{
			workingMove.x = x;
			workingMove.y = y;

			// This function assumes 'Up' and 'Right' are positive changes in index
			if (y < boardHeight - 1)
			{
				workingMove.direction = MoveDirection::Up;
				rankMoveForBoard(workingMove, workingBoard, potentialMoves);
			}
			if (x < boardWidth - 1)
			{
				workingMove.direction = MoveDirection::Right;
				rankMoveForBoard(workingMove, workingBoard, potentialMoves);
			}
		}
	}

	return potentialMoves;
}

Move MatchingGameExercise::calculateBestMoveForBoard(const Board& board)
{
	// Valid moves, ordered by calculated score in ascending order
	RankedMoves potentialMoves = calculateMovesForBoard(board);

	if (potentialMoves.size() > 0)
	{
		auto bestMoves = potentialMoves.rbegin();
		Move bestMove = bestMoves->second.front(); // Pick the first valid scoring move
		return bestMove;
	}
	else
	{
		// No scoring moves were found, but we cannot perform a swap if there are no matches
		// TODO: Alter function to return a bool to denote success
		throw std::logic_error("No valid moves can be performed");
	}
}
