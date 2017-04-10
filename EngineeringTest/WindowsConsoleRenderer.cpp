#include "WindowsConsoleRenderer.h"

#include "MatchingGameExercise.h"

#include <string>
#include <windows.h>

namespace
{

std::map<JewelKind, int> JewelKindToWindowsColorMap = {
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
	for (const auto& move : moves)
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
	int drawColor = JewelKindToWindowsColorMap[cellKind];
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

COORD convertGraphPointToConsoleCoordinate(float posX, float posY, float scaleX, float scaleY, float minExtent, float maxExtent, COORD cursorOffset)
{
	float range = maxExtent - minExtent;
	float plottedX = posX * scaleX;
	// Invert vertical values to print with 0, 0 as bottom-left coordinate
	float plottedY = (range - (posY - minExtent)) * scaleY;
	return { (SHORT)round(plottedX + cursorOffset.X), (SHORT)round(plottedY + cursorOffset.Y + 1) };
}

} // namespace

void WindowsConsoleRenderer::printBoardWithHighlightedMovesAndMatches(const Board& board, const std::vector<Move>& moves, const MatchedCellsCollection& highlightCells) const
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
			for (const auto& matches : highlightCells)
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

void populateCoordinatesForPath(float dX, float dY, float uX, float uY, float v, float a, float t, float boundingWidth, size_t pointsToPlot, std::vector<Vec2>& out_plottedPoints)
{
	// Derive a set of points from time 0..t
	float interval = t / (float)pointsToPlot;
	for (float i = 0.0f; i <= t; i += interval)
	{
		float xCoord = reflectValueBetweenBounds(uX * i + dX, 0, boundingWidth);
		float velocityAtTimeI = uY + a * i;
		float yCoord = dY + ((uY + (velocityAtTimeI - uY) * 0.5f) * i);
		out_plottedPoints.push_back({ xCoord, yCoord });
	}
}

void WindowsConsoleRenderer::printGraphWithPathAndTargetHeight(float height, Vec2 startPoint, Vec2 startVelocity, float verticalAcceleration, float boundingWidth)
{
	// Calculate some reasonable extents for the graph to draw
	float minExtent = min(startPoint.y, height);
	float maxExtent = max(startPoint.y, height);

	// Build a collection of points to render to the graph
	std::vector<Vec2> plottedCoordinates;
	int pointsToPlot = 100;
	plottedCoordinates.reserve(pointsToPlot);

	// Expand contents of tryCalculateXPositionAtHeight function to recreate intermediate calculated values
	float u = startVelocity.y;
	float uSq = u * u;
	float a = verticalAcceleration;
	float endVSq = uSq + 2 * a*(height - startPoint.y);
	bool targetReached = (endVSq >= 0);
	if (targetReached)
	{
		// Path intersects with target line, calculate accordingly
		float endV = sqrtf(endVSq);
		float averageVerticalVelocity = (u + (endV - u) * 0.5f);
		float t = (height - startPoint.y) / averageVerticalVelocity;
		if (t < 0)
		{
			// Path follows an arc in the opposite direction (sign was lost in endVSq above)
			t = (height - startPoint.y) / (u + (-endV - u) * 0.5f);
		}
		populateCoordinatesForPath(startPoint.x, startPoint.y, startVelocity.x, startVelocity.y, endV, verticalAcceleration, t, boundingWidth, pointsToPlot, plottedCoordinates);
	}
	else
	{
		// Path does not intersect with target line, attempt to render the outcome
		float targetHeight = 0;
		float endVSq = uSq + 2 * a*(targetHeight - startPoint.y);
		if (endVSq >= 0)
		{
			// Path intersects with the floor, attempt to render
			float endV = sqrtf(endVSq);
			float averageVerticalVelocity = (u + (endV - u) * 0.5f);
			float t = (height - startPoint.y) / averageVerticalVelocity;
			populateCoordinatesForPath(startPoint.x, startPoint.y, startVelocity.x, startVelocity.y, endV, verticalAcceleration, t, boundingWidth, pointsToPlot, plottedCoordinates);
		}
		else
		{
			// Path also does not reach the floor, pick some duration to follow the path before giving up
			float t = 5.0f;
			float endV = startVelocity.y + verticalAcceleration * t;
			populateCoordinatesForPath(startPoint.x, startPoint.y, startVelocity.x, startVelocity.y, endV, verticalAcceleration, t, boundingWidth, pointsToPlot, plottedCoordinates);
		}
	}

	for (const auto& entry : plottedCoordinates)
	{
		// Update bounding extents for rendering the graph
		maxExtent = max(entry.y, maxExtent);
		minExtent = min(entry.y, minExtent);
	}

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hConsole, &csbi);
	COORD cursorPosition = csbi.dwCursorPosition;
	float totalVerticalRange = (maxExtent - minExtent) + 1;
	int graphWidth = 50;
	int graphHeight = 30;
	if (totalVerticalRange > 0)
	{
		float xMagnitude = graphWidth / boundingWidth;
		float yMagnitude = graphHeight / totalVerticalRange;

		// Render target line
		COORD currentCursorPosition = convertGraphPointToConsoleCoordinate(0, height, xMagnitude, yMagnitude, minExtent, maxExtent, cursorPosition);
		SetConsoleCursorPosition(hConsole, { currentCursorPosition.X + 1, currentCursorPosition.Y });
		SetConsoleTextAttribute(hConsole, 0x0F);
		printf("%s", std::string(graphWidth, '-').c_str());

		// Render plotted points
		SetConsoleTextAttribute(hConsole, 0x07);
		for (const auto& entry : plottedCoordinates)
		{
			currentCursorPosition = convertGraphPointToConsoleCoordinate(entry.x, entry.y, xMagnitude, yMagnitude, minExtent, maxExtent, cursorPosition);
			SetConsoleCursorPosition(hConsole, currentCursorPosition);
			printf(".");
		}
		SetConsoleCursorPosition(hConsole, convertGraphPointToConsoleCoordinate(plottedCoordinates.front().x, plottedCoordinates.front().y, xMagnitude, yMagnitude, minExtent, maxExtent, cursorPosition));
		printf("o");
		if (targetReached)
		{
			SetConsoleTextAttribute(hConsole, 0x0E);
		}
		// Print a different character to denote the end point
		SetConsoleCursorPosition(hConsole, currentCursorPosition);
		printf("x");

		// Render bounding area
		SetConsoleTextAttribute(hConsole, 0x05);
		for (int i = 0; i < graphHeight + 2; i++)
		{
			SetConsoleCursorPosition(hConsole, { (SHORT)(cursorPosition.X), (SHORT)(cursorPosition.Y + i) });
			printf("|");
			SetConsoleCursorPosition(hConsole, { (SHORT)(cursorPosition.X + graphWidth + 1), (SHORT)(cursorPosition.Y + i) });
			printf("|");
		}

		// Move cursor down to the end of the graph
		cursorPosition.Y = cursorPosition.Y + graphHeight + 2;
		SetConsoleCursorPosition(hConsole, cursorPosition);
		SetConsoleTextAttribute(hConsole, 0x07);
	}
	else
	{
		printf("Unable to render graph, vertical range is zero");
	}
}
