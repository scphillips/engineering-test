#include "WindowsConsoleRenderer.h"

#include "BallGameExercise.h"
#include "MatchingGameExercise.h"
#include "RacingGameExercise.h"

#include <chrono>
#include <cstdlib>
#include <functional>

using Clock = std::chrono::steady_clock;

void runRacerTest(float updateTick, std::vector<Racer*>& racers, std::function<void(float, std::vector<Racer*>&)> exec)
{
	std::chrono::time_point<Clock> start = Clock::now();
	exec(updateTick, racers);
	std::chrono::time_point<Clock> end = Clock::now();
	std::chrono::milliseconds diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	printf("\nTime taken: %lldms, racers remaining: %d\n", diff.count(), racers.size());
}

int main(int argc, char** argv)
{
	srand(1);

	// ==========
	// Exercise 1
	//
	printf("# Exercise 1\n");
	WindowsConsoleRenderer renderer;

	MatchingGameExercise matchingGame;
	unsigned int boardWidth = 8;
	unsigned int boardHeight = 8;

	printf("\nBeginning a matching game with a %dx%d size board\n", boardWidth, boardHeight);
	Board gameBoard = matchingGame.beginGame(boardWidth, boardHeight);
	printf("Calculating most optimal moves\n\n");
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

		// Recreate moves and render each state of the board to show cascading matches
		Move bestMove = bestMoves->second.front();
		printf("\nResults after move (switching [x:%d y:%d] with %s):\n", bestMove.x, bestMove.y, moveDirectionToString(bestMove.direction).c_str());
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
	printf("\n# Exercise 2\n");
	float targetHeight = 75.0f;
	Vec2 startPoint = { 20.0f, 80.0f };
	Vec2 startVelocity = { 50.0f, 30.0f };
	float gravity = -9.807f;
	float boundsWidth = 100.0f;
	float finalXCoordinate;
	printf("\nAttempting to find the intersection point of a path with the following parameters:\n");
	printf("Target height: %g\nStarting point: [x:%g y:%g]\nStarting velocity: [x:%g y:%g]\n", targetHeight, startPoint.x, startPoint.y, startVelocity.x, startVelocity.y);
	printf("Gravity: %g\nBounding walls: [Left:%g Right:%g]\n", gravity, 0.0f, boundsWidth);
	if (tryCalculateXPositionAtHeight(targetHeight, startPoint, startVelocity, gravity, boundsWidth, finalXCoordinate))
	{
		printf("\nFinal coordinate: [x:%g y:%g]\n", finalXCoordinate, targetHeight);
	}
	else
	{
		printf("\nPath does not intersect with line 'h'.\n");
	}
	printf("Start point 'o', end point 'x':\n");
	renderer.printGraphWithPathAndTargetHeight(targetHeight, startPoint, startVelocity, gravity, boundsWidth);

	// ==========
	// Exercise 3
	printf("\n# Exercise 3\n");
	int racerCount = 1000;

	printf("\nPerforming original updateRacers() with %d racers (please wait)", racerCount);
	std::vector<Racer*> testRacersOriginal = createRacerCollection(racerCount);
	float updateTick = 1.0f / 60.0f;
	runRacerTest(updateTick, testRacersOriginal, updateRacers);

	printf("\nPerforming new updateRacersV2() with %d racers", racerCount);
	std::vector<Racer*> testRacersNew = createRacerCollection(racerCount);
	runRacerTest(updateTick, testRacersNew, updateRacersV2);

	// TODO: Implement Racer class and set up testbed to compare results more scientifically
	bool match = (testRacersOriginal.size() == testRacersNew.size());
	if (match == true)
	{
		for (size_t i = 0; i < testRacersOriginal.size(); i++)
		{
			const auto* racerOriginal = testRacersOriginal[i];
			const auto* racerNew = testRacersNew[i];
			if (racerOriginal->isAlive() != racerNew->isAlive()
				|| racerOriginal->isCollidable() != racerNew->isCollidable())
			{
				match = false;
				break;
			}
		}
	}
	printf("\nOutput from both functions %s.\n", (match == true) ? "match" : "do not match");

	// Cleanup racer allocations
	for (const auto* racer : testRacersOriginal)
	{
		delete racer;
	}
	for (const auto* racer : testRacersNew)
	{
		delete racer;
	}

	system("pause");
}
