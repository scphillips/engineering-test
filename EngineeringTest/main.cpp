#include "WindowsConsoleRenderer.h"

#include "BallGameExercise.h"
#include "MatchingGameExercise.h"
#include "RacingGameExercise.h"

#include <chrono>
#include <cstdlib>

using Clock = std::chrono::steady_clock;

int main(int argc, char** argv)
{
	srand(1);

	// ==========
	// Exercise 1
	//
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

	// ==========
	// Exercise 2
	float targetHeight = 75.0f;
	Vec2 startPoint = { 20.0f, 30.0f };
	Vec2 startVelocity = { 50.0f, 30.0f };
	float gravity = -9.807f;
	float boundsWidth = 100.0f;
	float finalXCoordinate;
	if (tryCalculateXPositionAtHeight(targetHeight, startPoint, startVelocity, gravity, boundsWidth, finalXCoordinate))
	{
		printf("\nFinal coordinate: [x:%f y:%f]\n", finalXCoordinate, targetHeight);
	}
	else
	{
		printf("\nUnable to resolve formula\n");
	}

	// ==========
	// Exercise 3
	int racerCount = 1000;
	
	std::vector<Racer*> oldTestCollection = createRacerCollection(racerCount);
	printf("\nPerforming original updateRacers() with %d racers", racerCount);
	std::chrono::time_point<Clock> start = Clock::now();
	updateRacers(1.0f / 60.0f, oldTestCollection);
	std::chrono::time_point<Clock> end = Clock::now();
	std::chrono::milliseconds diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	printf("\nTime taken: %lldms\n", diff.count());

	std::vector<Racer*> newTestCollection = createRacerCollection(racerCount);
	printf("\nPerforming new updateRacersV2() with %d racers", racerCount);
	start = Clock::now();
	updateRacersV2(1.0f / 60.0f, newTestCollection);
	end = Clock::now();
	diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	printf("\nTime taken: %lldms\n", diff.count());

	printf("\nNumber of racers remaining - old: %d, new: %d\n", oldTestCollection.size(), newTestCollection.size());
	// TODO: Implement Racer class and set up testbed to compare results more scientifically

	system("pause");
}
