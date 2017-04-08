#pragma once

#include "MatchingGameDecl.h"

class WindowsConsoleRenderer
{
public:
	void printBoardWithHighlightedMovesAndMatches(const Board& board, std::vector<Move> moves, MatchedCellsCollection highlightCells) const;
};
