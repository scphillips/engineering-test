#pragma once

#include "MatchingGameDecl.h"

class MatchingGameExercise
{
public:
	Board beginGame(int width, int height);
	RankedMoves calculateMovesForBoard(const Board& board);
	Move calculateBestMoveForBoard(const Board& board);
};
