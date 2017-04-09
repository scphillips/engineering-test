#pragma once

#include "MatchingGameDecl.h"
#include "BallGameExercise.h"

class WindowsConsoleRenderer
{
public:
	void printBoardWithHighlightedMovesAndMatches(const Board& board, std::vector<Move> moves, MatchedCellsCollection highlightCells) const;
	void printGraphWithPathAndTargetHeight(float height, Vec2 startPoint, Vec2 startVelocity, float verticalAcceleration, float boundingWidth);
};
