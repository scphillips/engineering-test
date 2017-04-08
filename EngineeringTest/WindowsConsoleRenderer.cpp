#include "WindowsConsoleRenderer.h"

#include "MatchingGameExercise.h"

#include <string>
#include <windows.h>

namespace
{

std::map<JewelKind, int> WindowsColorMap = {
	{ Empty, 0x7 },
	{ Red, 0xC0 },
	{ Orange, 0x60 },
	{ Yellow, 0xE0 },
	{ Green, 0xA0 },
	{ Blue, 0x90 },
	{ Indigo, 0xD0 },
	{ Violet, 0x50 }
};

void findMoveDirectionsAffectingCell(int x, int y, const std::vector<Move>& moves, bool& out_up, bool& out_down, bool& out_left, bool& out_right)
{
	int targetX, targetY;
	for (auto move : moves)
	{
		getIndexAfterMove(move, targetX, targetY);
		bool isMoveSrc = (move.x == x && move.y == y);
		bool isMoveDst = (targetX == x && targetY == y);
		if (isMoveSrc || isMoveDst)
		{
			out_up = ((move.direction == Up && isMoveSrc) || (move.direction == Down && isMoveDst)) ? true : out_up;
			out_down = ((move.direction == Down && isMoveSrc) || (move.direction == Up && isMoveDst)) ? true : out_down;
			out_left = ((move.direction == Left && isMoveSrc) || (move.direction == Right && isMoveDst)) ? true : out_left;
			out_right = ((move.direction == Right && isMoveSrc) || (move.direction == Left && isMoveDst)) ? true : out_right;
		}
	}
}

void printCellPadding(HANDLE hConsole, const Board& board, const std::vector<Move>& moves, int x, int y, int row, bool highlighted)
{
	JewelKind cellKind = board.getJewel(x, y);
	int drawColor = WindowsColorMap[cellKind];
	bool up, down, left, right;
	findMoveDirectionsAffectingCell(x, y, moves, up, down, left, right);
	std::string lhs = "  ";
	std::string mid = " ";
	std::string rhs = "  ";
	if (row == 0 && up == true)
	{
		mid = "V";
	}
	else if (row == 1)
	{
		if (left == true) lhs = "->";
		JewelKind value = board.getJewel(x, y);
		mid = std::to_string(value);
		if (right == true) rhs = "<-";
	}
	else if (row == 2 && down == true)
	{
		mid = "^";
	}
	SetConsoleTextAttribute(hConsole, drawColor | 0x0F);
	printf("%s ", lhs.c_str());
	if (row == 1 && !highlighted)
	{
		SetConsoleTextAttribute(hConsole, drawColor);
	}
	printf("%s", mid.c_str());
	SetConsoleTextAttribute(hConsole, drawColor | 0x0F);
	printf(" %s", rhs.c_str());
}

} // namespace

void WindowsConsoleRenderer::printBoardWithHighlightedMovesAndMatches(const Board& board, std::vector<Move> moves, MatchedCellsCollection highlightCells) const
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hConsole, &csbi);
	int minWidth = board.getWidth() * 7 + 1;
	if (csbi.dwSize.X < minWidth)
	{
		csbi.dwSize.X = minWidth;
		SetConsoleScreenBufferSize(hConsole, csbi.dwSize);
		printf("\n");
	}

	for (int y = board.getHeight() - 1; y >= 0; y--)
	{
		for (int x = 0; x < board.getWidth(); x++)
		{
			printCellPadding(hConsole, board, moves, x, y, 0, false);
		}
		printf("\n");
		for (int x = 0; x < board.getWidth(); x++)
		{
			bool highlighted = false;
			for (auto matches : highlightCells)
			{
				highlighted = (matches.count({ x, y }) > 0) ? true : highlighted;
			}
			printCellPadding(hConsole, board, moves, x, y, 1, highlighted);
		}
		printf("\n");
		for (int x = 0; x < board.getWidth(); x++)
		{
			printCellPadding(hConsole, board, moves, x, y, 2, false);
		}
		printf("\n");
	}
	SetConsoleTextAttribute(hConsole, 0x07);
}
