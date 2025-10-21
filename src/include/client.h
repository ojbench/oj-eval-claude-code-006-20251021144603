#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>
#include <vector>
#include <queue>
#include <set>
#include <algorithm>
#include <random>

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.

// You MUST NOT use any other external variables except for rows, columns and total_mines.

// AI game state
enum CellState {
  UNKNOWN = 0,
  SAFE = 1,
  MINE = 2,
  VISITED = 3
};

std::vector<std::vector<CellState>> cell_states;
std::vector<std::vector<int>> cell_numbers;
std::vector<std::pair<int, int>> frontier_cells;
std::mt19937 rng(12345);  // Fixed seed for reproducibility

/**
 * @brief The definition of function Execute(int, int, bool)
 *
 * @details This function is designed to take a step when player the client's (or player's) role, and the implementation
 * of it has been finished by TA. (I hope my comments in code would be easy to understand T_T) If you do not understand
 * the contents, please ask TA for help immediately!!!
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 * @param type The type of operation to a certain block.
 * If type == 0, we'll execute VisitBlock(row, column).
 * If type == 1, we'll execute MarkMine(row, column).
 * If type == 2, we'll execute AutoExplore(row, column).
 * You should not call this function with other type values.
 */
void Execute(int r, int c, int type);

/**
 * @brief The definition of function InitGame()
 *
 * @details This function is designed to initialize the game. It should be called at the beginning of the game, which
 * will read the scale of the game map and the first step taken by the server (see README).
 */
void InitGame() {
  // Initialize AI state
  cell_states.assign(rows, std::vector<CellState>(columns, UNKNOWN));
  cell_numbers.assign(rows, std::vector<int>(columns, -1));
  frontier_cells.clear();

  // Read first move coordinates and execute
  int first_row, first_column;
  std::cin >> first_row >> first_column;
  Execute(first_row, first_column, 0);
}

/**
 * @brief The definition of function ReadMap()
 *
 * @details This function is designed to read the game map from stdin when playing the client's (or player's) role.
 * Since the client (or player) can only get the limited information of the game map, so if there is a 3 * 3 map as
 * above and only the block (2, 0) has been visited, the stdin would be
 *     ???
 *     12?
 *     01?
 */
void ReadMap() {
  frontier_cells.clear();

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      char cell;
      std::cin >> cell;

      if (cell >= '0' && cell <= '8') {
        // Visited numbered cell
        cell_numbers[i][j] = cell - '0';
        cell_states[i][j] = VISITED;
        frontier_cells.push_back({i, j});
      } else if (cell == 'X') {
        // Visited mine (game over scenario)
        cell_states[i][j] = MINE;
      } else if (cell == '@') {
        // Marked mine
        cell_states[i][j] = MINE;
      } else {
        // Unknown cell ('?')
        if (cell_states[i][j] == VISITED) {
          // This was previously visited but now shows as '?'
          // This happens in victory state where mines are revealed
          cell_states[i][j] = UNKNOWN;
        }
      }
    }
  }
}

/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
// Helper function to count unknown neighbors around a cell
int countUnknownNeighbors(int r, int c) {
  int count = 0;
  for (int dr = -1; dr <= 1; dr++) {
    for (int dc = -1; dc <= 1; dc++) {
      if (dr == 0 && dc == 0) continue;
      int nr = r + dr, nc = c + dc;
      if (nr >= 0 && nr < rows && nc >= 0 && nc < columns) {
        if (cell_states[nr][nc] == UNKNOWN) count++;
      }
    }
  }
  return count;
}

// Helper function to count marked mine neighbors around a cell
int countMineNeighbors(int r, int c) {
  int count = 0;
  for (int dr = -1; dr <= 1; dr++) {
    for (int dc = -1; dc <= 1; dc++) {
      if (dr == 0 && dc == 0) continue;
      int nr = r + dr, nc = c + dc;
      if (nr >= 0 && nr < rows && nc >= 0 && nc < columns) {
        if (cell_states[nr][nc] == MINE) count++;
      }
    }
  }
  return count;
}

// Helper function to get unknown neighbor coordinates
std::vector<std::pair<int, int>> getUnknownNeighbors(int r, int c) {
  std::vector<std::pair<int, int>> neighbors;
  for (int dr = -1; dr <= 1; dr++) {
    for (int dc = -1; dc <= 1; dc++) {
      if (dr == 0 && dc == 0) continue;
      int nr = r + dr, nc = c + dc;
      if (nr >= 0 && nr < rows && nc >= 0 && nc < columns) {
        if (cell_states[nr][nc] == UNKNOWN) {
          neighbors.push_back({nr, nc});
        }
      }
    }
  }
  return neighbors;
}

void Decide() {
  // Step 1: Try auto-explore first
  for (auto [r, c] : frontier_cells) {
    int mine_count = countMineNeighbors(r, c);
    if (cell_numbers[r][c] == mine_count) {
      Execute(r, c, 2);  // Auto explore
      return;
    }
  }

  // Step 2: Find one obvious mine or safe cell
  for (auto [r, c] : frontier_cells) {
    int unknown_count = countUnknownNeighbors(r, c);
    int mine_count = countMineNeighbors(r, c);

    // If all mines are found, all remaining unknown cells are safe
    if (cell_numbers[r][c] == mine_count && unknown_count > 0) {
      auto neighbors = getUnknownNeighbors(r, c);
      if (!neighbors.empty()) {
        Execute(neighbors[0].first, neighbors[0].second, 0);  // Visit safe cell
        return;
      }
    }

    // If remaining unknown cells equal remaining mines, all are mines
    if (cell_numbers[r][c] - mine_count == unknown_count && unknown_count > 0) {
      auto neighbors = getUnknownNeighbors(r, c);
      if (!neighbors.empty()) {
        Execute(neighbors[0].first, neighbors[0].second, 1);  // Mark mine
        return;
      }
    }
  }

  // Step 3: Pick the first unknown cell (very simple fallback)
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (cell_states[i][j] == UNKNOWN) {
        Execute(i, j, 0);  // Visit
        return;
      }
    }
  }
}

#endif