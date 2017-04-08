#include "WindowsConsoleRenderer.h"
#include "MatchingGameExercise.h"

#include <cstdlib>

int main(int argc, char** argv)
{
	srand(1);

	WindowsConsoleRenderer renderer;

	MatchingGameExercise matchingGame;
	unsigned int boardWidth = 8;
	unsigned int boardHeight = 8;
	Board gameBoard = matchingGame.beginGame(boardWidth, boardHeight);
	RankedMoves potentialMoves = matchingGame.calculateMovesForBoard(gameBoard);

	if (!potentialMoves.empty())
	{
		auto bestMoves = potentialMoves.rbegin();
		printf("Best moves with a score of %d:\n", bestMoves->first);
		for (const auto& move : bestMoves->second)
		{
			printf("    Switching [x:%d y:%d] with %s\n", move.x, move.y, moveDirectionToString(move.direction).c_str());
		}
		renderer.printBoardWithHighlightedMovesAndMatches(gameBoard, bestMoves->second, {});
		printf("\nResults:\n");

		// Recreate moves and render each state of the board to show cascading matches
		Move bestMove = bestMoves->second.front();
		performMoveForBoard(bestMove, gameBoard);
		MatchedCellsCollection results = findMatchesAfterMoveForBoard(bestMove, gameBoard);
		renderer.printBoardWithHighlightedMovesAndMatches(gameBoard, {}, results);
		resolveMatchesForBoard(results, gameBoard);
		repopulateBoardAfterMatches(results, gameBoard);

		int cascadeChain = 0;
		auto& cursor = resolveCascadingMatches(gameBoard);
		while (!cursor.empty())
		{
			printf("\nCascade chain: %d\n", ++cascadeChain);
			renderer.printBoardWithHighlightedMovesAndMatches(gameBoard, {}, cursor);
			repopulateBoardAfterMatches(cursor, gameBoard);
			cursor = resolveCascadingMatches(gameBoard);
		}
		printf("\nFinal state:\n");
		renderer.printBoardWithHighlightedMovesAndMatches(gameBoard, {}, {});
	}
	else
	{
		printf("No valid moves could be found\n\n");
		renderer.printBoardWithHighlightedMovesAndMatches(gameBoard, {}, {});
	}
}
