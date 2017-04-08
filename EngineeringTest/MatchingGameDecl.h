#pragma once

#include <map>
#include <set>
#include <vector>

static const int NumberOfColorsToMatch = 3;

enum JewelKind
{
	Empty,
	Red,
	Orange,
	Yellow,
	Green,
	Blue,
	Indigo,
	Violet
};

enum MoveDirection
{
	Up,
	Down,
	Left,
	Right
};

struct Move
{
	int x;
	int y;
	MoveDirection direction;
};

struct BoardCell
{
	BoardCell(int newX, int newY) :
		x(newX),
		y(newY)
	{
	}

	int x;
	int y;
};

inline bool operator < (const BoardCell& lhs, const BoardCell& rhs)
{
	return (lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y));
}

using BoardCellCollection = std::set<BoardCell>;
using MatchedCellsCollection = std::vector<BoardCellCollection>;
using RankedMoves = std::map<int, std::vector<Move>>;

class Board
{
public:
	Board(int width, int height) :
		m_width(width),
		m_height(height)
	{
		m_cells.resize(width * height);
	}

	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }

	JewelKind getJewel(int x, int y) const { return m_cells[y*m_width + x]; }
	void setJewel(int x, int y, JewelKind kind) { m_cells[y*m_width + x] = kind; }

private:
	std::vector<JewelKind> m_cells;
	int m_width;
	int m_height;
};

inline std::string moveDirectionToString(MoveDirection direction)
{
	switch (direction)
	{
	case MoveDirection::Up:
		return "Up";
	case MoveDirection::Down:
		return "Down";
	case MoveDirection::Left:
		return "Left";
	case MoveDirection::Right:
		return "Right";
	default:
		return "Unknown";
	}
}

inline bool getIndexAfterMove(const Move& move, int& out_x, int& out_y)
{
	out_x = move.x;
	out_y = move.y;

	// This function assumes 'Up' and 'Right' are positive changes in index
	switch (move.direction)
	{
	case MoveDirection::Up:
		out_y += 1;
		break;
	case MoveDirection::Down:
		out_y -= 1;
		break;
	case MoveDirection::Left:
		out_x -= 1;
		break;
	case MoveDirection::Right:
		out_x += 1;
		break;
	default:
		return false;
	}

	return true;
}

inline bool performMoveForBoard(const Move& move, Board& out_board)
{
	int targetX, targetY;
	bool isValidMove = getIndexAfterMove(move, targetX, targetY);
	if (isValidMove)
	{
		JewelKind jewelKindSrc = out_board.getJewel(move.x, move.y);
		JewelKind jewelKindDst = out_board.getJewel(targetX, targetY);
		// Perform the swap
		out_board.setJewel(move.x, move.y, jewelKindDst);
		out_board.setJewel(targetX, targetY, jewelKindSrc);
		return true;
	}
	else
	{
		// Received an invalid move
		return false;
	}
}

inline BoardCellCollection findNewAdjacentCells(const BoardCell& currentCell, const Board& board, const BoardCellCollection& visitedCells)
{
	BoardCellCollection adjacentCells;
	if (currentCell.x > 0)
	{
		adjacentCells.insert(BoardCell(currentCell.x - 1, currentCell.y));
	}
	if (currentCell.y > 0)
	{
		adjacentCells.insert(BoardCell(currentCell.x, currentCell.y - 1));
	}
	if (currentCell.x < board.getWidth() - 1)
	{
		adjacentCells.insert(BoardCell(currentCell.x + 1, currentCell.y));
	}
	if (currentCell.y < board.getHeight() - 1)
	{
		adjacentCells.insert(BoardCell(currentCell.x, currentCell.y + 1));
	}
	return adjacentCells;
}

inline void addMatchingCellsToCollection(const BoardCell& currentCell, const Board& board, BoardCellCollection& out_matchedCells, BoardCellCollection& out_visitedCells)
{
	BoardCellCollection cellsToTest = findNewAdjacentCells(currentCell, board, out_visitedCells);
	JewelKind kindToMatch = board.getJewel(currentCell.x, currentCell.y);
	if (kindToMatch != Empty)
	{
		for (auto adjacentCell : cellsToTest)
		{
			if (out_visitedCells.count(adjacentCell) == 0)
			{
				out_visitedCells.insert(adjacentCell);
				if (board.getJewel(adjacentCell.x, adjacentCell.y) == kindToMatch)
				{
					// Found a match, recurse to search for additional matches
					out_matchedCells.insert(adjacentCell);
					addMatchingCellsToCollection(adjacentCell, board, out_matchedCells, out_visitedCells);
				}
			}
		}
	}
}

inline void repopulateBoardAfterMatches(MatchedCellsCollection matchedGroups, Board& out_board)
{
	// Optional check: once jewels have been matched, the board may repopulate and cascade for more points
	// Remap collection into BoardCellCollection, sorted by row and column in ascending order
	BoardCellCollection matchedCells;
	for (auto collection : matchedGroups)
	{
		for (auto cell : collection)
		{
			matchedCells.insert(cell);
		}
	}

	// Traverse the board where matches were made, moving jewels down into empty spaces
	// Begin from the top of the board to avoid shifting matched cells still to be resolved
	for (auto it = matchedCells.rbegin(); it != matchedCells.rend(); ++it)
	{
		auto cell = *it;
		for (int y = cell.y; y < out_board.getHeight() - 1; y++)
		{
			out_board.setJewel(cell.x, y, out_board.getJewel(cell.x, y + 1));
		}
		out_board.setJewel(cell.x, out_board.getHeight() - 1, JewelKind::Empty);
	}
}

inline void resolveMatchesForBoard(const MatchedCellsCollection& potentialMatches, Board& out_board)
{
	for (auto matchedCells : potentialMatches)
	{
		for (auto cell : matchedCells)
		{
			out_board.setJewel(cell.x, cell.y, Empty);
		}
	}
}

inline MatchedCellsCollection resolveCascadingMatches(Board& out_board)
{
	// Depending on the behaviour of repopulating the board, we could intelligently check areas which may have new matches.
	// This code simply scans the entire board
	BoardCellCollection cellsToCheck;
	for (int y = 0; y < out_board.getHeight(); y++)
	{
		for (int x = 0; x < out_board.getWidth(); x++)
		{
			cellsToCheck.insert(BoardCell(x, y));
		}
	}
	MatchedCellsCollection cascadeMatches;
	for (auto cell : cellsToCheck)
	{
		BoardCellCollection matchedCells;
		addMatchingCellsToCollection(cell, out_board, matchedCells, BoardCellCollection());
		if (matchedCells.size() >= NumberOfColorsToMatch)
		{
			cascadeMatches.push_back(matchedCells);
		}
	}

	return cascadeMatches;
}

inline MatchedCellsCollection findMatchesAfterMoveForBoard(const Move& move, const Board& board)
{
	// Run visit function for source and destination cells to check for matches
	BoardCell currentCell(move.x, move.y);
	BoardCellCollection matchedCellsSrc;
	addMatchingCellsToCollection(currentCell, board, matchedCellsSrc, BoardCellCollection());

	// This code assumes there were no matches before the move. In situations where this is not the case,
	//  it may incorrectly double the user's score by matching the same cells twice
	getIndexAfterMove(move, currentCell.x, currentCell.y);
	BoardCellCollection matchedCellsDst;
	addMatchingCellsToCollection(currentCell, board, matchedCellsDst, BoardCellCollection());

	MatchedCellsCollection potentialMatches = { matchedCellsSrc, matchedCellsDst };
	MatchedCellsCollection results;
	for (auto matchedCells : potentialMatches)
	{
		if (matchedCells.size() >= NumberOfColorsToMatch)
		{
			results.push_back(matchedCells);
		}
	}
	return results;
}

inline int calculateScoreAfterMoveForBoard(const Move& move, const Board& board)
{
	int totalScore = 0;
	Board workingBoard = board;
	if (performMoveForBoard(move, workingBoard))
	{
		MatchedCellsCollection results = findMatchesAfterMoveForBoard(move, workingBoard);
		resolveMatchesForBoard(results, workingBoard);
		repopulateBoardAfterMatches(results, workingBoard);

		// Add up total cell count for matching entries in each collection
		// Additional rules such as multipliers could be added here
		auto& cursor = results;
		while (!cursor.empty())
		{
			BoardCellCollection uniqueCells;
			// Extract into BoardCellCollection to only count each matching cell once
			for (auto match : cursor)
			{
				for (auto cell : match)
				{
					uniqueCells.insert(cell);
				}
			}
			totalScore += (int)uniqueCells.size();

			cursor = resolveCascadingMatches(workingBoard);
			repopulateBoardAfterMatches(cursor, workingBoard);
		}
	}
	else
	{
		throw std::runtime_error("Invalid move when traversing board");
	}
	return totalScore;
}

inline void rankMoveForBoard(const Move& move, const Board& board, RankedMoves& out_ranking)
{
	int score = calculateScoreAfterMoveForBoard(move, board);
	if (score > 0)
	{
		// Found a valid scoring move, add it to the potential moves
		auto existingMoves = out_ranking.find(score);
		if (existingMoves == out_ranking.end())
		{
			// First move with this score result, create new entry
			RankedMoves::value_type::second_type entries;
			entries.push_back(move);
			out_ranking.insert(std::make_pair(score, entries));
		}
		else
		{
			// Other moves also have this score, add to collection
			existingMoves->second.push_back(move);
		}
	}
}
