#ifndef SERVER_H
#define SERVER_H

#include <cstdlib>
#include <iostream>
#include <vector>
#include <queue>

/*
 * You may need to define some global variables for the information of the game map here.
 * Although we don't encourage to use global variables in real cpp projects, you may have to use them because the use of
 * class is not taught yet. However, if you are member of A-class or have learnt the use of cpp class, member functions,
 * etc., you're free to modify this structure.
 */
int rows;         // The count of rows of the game map. You MUST NOT modify its name.
int columns;      // The count of columns of the game map. You MUST NOT modify its name.
int total_mines;  // The count of mines of the game map. You MUST NOT modify its name. You should initialize this
                  // variable in function InitMap. It will be used in the advanced task.
int game_state;  // The state of the game, 0 for continuing, 1 for winning, -1 for losing. You MUST NOT modify its name.

// Game map data structures
std::vector<std::vector<bool>> is_mine;        // True if cell contains mine
std::vector<std::vector<bool>> is_visited;     // True if cell has been visited
std::vector<std::vector<bool>> is_marked;      // True if cell has been marked
std::vector<std::vector<int>> mine_count;      // Number of adjacent mines for each cell
int visit_count = 0;                           // Number of visited non-mine cells
int marked_mine_count = 0;                     // Number of correctly marked mines

/**
 * @brief The definition of function InitMap()
 *
 * @details This function is designed to read the initial map from stdin. For example, if there is a 3 * 3 map in which
 * mines are located at (0, 1) and (1, 2) (0-based), the stdin would be
 *     3 3
 *     .X.
 *     ...
 *     ..X
 * where X stands for a mine block and . stands for a normal block. After executing this function, your game map
 * would be initialized, with all the blocks unvisited.
 */
void InitMap() {
  std::cin >> rows >> columns;

  // Initialize game state
  game_state = 0;
  visit_count = 0;
  marked_mine_count = 0;
  total_mines = 0;

  // Resize vectors to match map dimensions
  is_mine.resize(rows, std::vector<bool>(columns, false));
  is_visited.resize(rows, std::vector<bool>(columns, false));
  is_marked.resize(rows, std::vector<bool>(columns, false));
  mine_count.resize(rows, std::vector<int>(columns, 0));

  // Read the map
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      char cell;
      std::cin >> cell;
      if (cell == 'X') {
        is_mine[i][j] = true;
        total_mines++;
      }
      // Debug: Uncomment to check input reading
      // std::cerr << "Read " << cell << " at (" << i << "," << j << ")" << std::endl;
    }
  }

  // Calculate mine counts for each cell
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (!is_mine[i][j]) {
        int count = 0;
        // Check all 8 directions
        for (int di = -1; di <= 1; di++) {
          for (int dj = -1; dj <= 1; dj++) {
            if (di == 0 && dj == 0) continue;
            int ni = i + di, nj = j + dj;
            if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && is_mine[ni][nj]) {
              count++;
            }
          }
        }
        mine_count[i][j] = count;
        }
    }
  }
}

/**
 * @brief The definition of function VisitBlock(int, int)
 *
 * @details This function is designed to visit a block in the game map. We take the 3 * 3 game map above as an example.
 * At the beginning, if you call VisitBlock(0, 0), the return value would be 0 (game continues), and the game map would
 * be
 *     1??
 *     ???
 *     ???
 * If you call VisitBlock(0, 1) after that, the return value would be -1 (game ends and the players loses) , and the
 * game map would be
 *     1X?
 *     ???
 *     ???
 * If you call VisitBlock(0, 2), VisitBlock(2, 0), VisitBlock(1, 2) instead, the return value of the last operation
 * would be 1 (game ends and the player wins), and the game map would be
 *     1@1
 *     122
 *     01@
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 *
 * @note You should edit the value of game_state in this function. Precisely, edit it to
 *    0  if the game continues after visit that block, or that block has already been visited before.
 *    1  if the game ends and the player wins.
 *    -1 if the game ends and the player loses.
 *
 * @note For invalid operation, you should not do anything.
 */
void VisitBlock(int r, int c) {
  // Check if coordinates are valid
  if (r < 0 || r >= rows || c < 0 || c >= columns) return;

  // Check if already visited or marked
  if (is_visited[r][c] || is_marked[r][c]) return;

  // If visiting a mine, game over
  if (is_mine[r][c]) {
    game_state = -1;
    return;
  }

  // Visit the cell
  is_visited[r][c] = true;
  visit_count++;

  // If mine count is 0, automatically visit all surrounding cells
  if (mine_count[r][c] == 0) {
    std::queue<std::pair<int, int>> q;
    q.push({r, c});

    while (!q.empty()) {
      auto [cur_r, cur_c] = q.front();
      q.pop();

      // Check all 8 neighbors
      for (int di = -1; di <= 1; di++) {
        for (int dj = -1; dj <= 1; dj++) {
          if (di == 0 && dj == 0) continue;
          int nr = cur_r + di, nc = cur_c + dj;

          // Check bounds
          if (nr >= 0 && nr < rows && nc >= 0 && nc < columns) {
            // Only visit non-mine, unvisited, unmarked cells
            if (!is_mine[nr][nc] && !is_visited[nr][nc] && !is_marked[nr][nc]) {
              is_visited[nr][nc] = true;
              visit_count++;

              // If this cell also has 0 mines, add to queue
              if (mine_count[nr][nc] == 0) {
                q.push({nr, nc});
              }
            }
          }
        }
      }
    }
  }

  // Check win condition: all non-mine cells visited
  int total_non_mines = rows * columns - total_mines;
  if (visit_count == total_non_mines) {
    game_state = 1;
  }
}

/**
 * @brief The definition of function MarkMine(int, int)
 *
 * @details This function is designed to mark a mine in the game map.
 * If the block being marked is a mine, show it as "@".
 * If the block being marked isn't a mine, END THE GAME immediately. (NOTE: This is not the same rule as the real
 * game) And you don't need to
 *
 * For example, if we use the same map as before, and the current state is:
 *     1?1
 *     ???
 *     ???
 * If you call MarkMine(0, 1), you marked the right mine. Then the resulting game map is:
 *     1@1
 *     ???
 *     ???
 * If you call MarkMine(1, 0), you marked the wrong mine(There's no mine in grid (1, 0)).
 * The game_state would be -1 and game ends immediately. The game map would be:
 *     1?1
 *     X??
 *     ???
 * This is different from the Minesweeper you've played. You should beware of that.
 *
 * @param r The row coordinate (0-based) of the block to be marked.
 * @param c The column coordinate (0-based) of the block to be marked.
 *
 * @note You should edit the value of game_state in this function. Precisely, edit it to
 *    0  if the game continues after visit that block, or that block has already been visited before.
 *    1  if the game ends and the player wins.
 *    -1 if the game ends and the player loses.
 *
 * @note For invalid operation, you should not do anything.
 */
void MarkMine(int r, int c) {
  // Check if coordinates are valid
  if (r < 0 || r >= rows || c < 0 || c >= columns) return;

  // Check if already visited or marked
  if (is_visited[r][c] || is_marked[r][c]) return;

  // Mark the cell
  is_marked[r][c] = true;

  // If marking a non-mine, game over
  if (!is_mine[r][c]) {
    game_state = -1;
    return;
  }

  // Correctly marked a mine
  marked_mine_count++;
}

/**
 * @brief The definition of function AutoExplore(int, int)
 *
 * @details This function is designed to auto-visit adjacent blocks of a certain block.
 * See README.md for more information
 *
 * For example, if we use the same map as before, and the current map is:
 *     ?@?
 *     ?2?
 *     ??@
 * Then auto explore is available only for block (1, 1). If you call AutoExplore(1, 1), the resulting map will be:
 *     1@1
 *     122
 *     01@
 * And the game ends (and player wins).
 */
void AutoExplore(int r, int c) {
  // Check if coordinates are valid
  if (r < 0 || r >= rows || c < 0 || c >= columns) return;

  // Auto explore only works on visited non-mine grids (showing numbers)
  if (!is_visited[r][c] || is_mine[r][c]) return;

  // Count marked mines around the target cell
  int marked_count = 0;
  std::vector<std::pair<int, int>> unmarked_non_mines;

  for (int di = -1; di <= 1; di++) {
    for (int dj = -1; dj <= 1; dj++) {
      if (di == 0 && dj == 0) continue;
      int nr = r + di, nc = c + dj;

      if (nr >= 0 && nr < rows && nc >= 0 && nc < columns) {
        if (is_marked[nr][nc]) {
          marked_count++;
        } else if (!is_mine[nr][nc] && !is_visited[nr][nc]) {
          unmarked_non_mines.push_back({nr, nc});
        }
      }
    }
  }

  // If marked mines count equals mine count, visit all unmarked non-mine neighbors
  if (marked_count == mine_count[r][c]) {
    for (auto [nr, nc] : unmarked_non_mines) {
      VisitBlock(nr, nc);
      if (game_state != 0) break; // Stop if game ends
    }
  }
}

/**
 * @brief The definition of function ExitGame()
 *
 * @details This function is designed to exit the game.
 * It outputs a line according to the result, and a line of two integers, visit_count and marked_mine_count,
 * representing the number of blocks visited and the number of marked mines taken respectively.
 *
 * @note If the player wins, we consider that ALL mines are correctly marked.
 */
void ExitGame() {
  if (game_state == 1) {
    std::cout << "YOU WIN!" << std::endl;
    // In victory, all mines are considered correctly marked
    marked_mine_count = total_mines;
  } else {
    std::cout << "GAME OVER!" << std::endl;
  }

  std::cout << visit_count << " " << marked_mine_count << std::endl;
  exit(0);  // Exit the game immediately
}

/**
 * @brief The definition of function PrintMap()
 *
 * @details This function is designed to print the game map to stdout. We take the 3 * 3 game map above as an example.
 * At the beginning, if you call PrintMap(), the stdout would be
 *    ???
 *    ???
 *    ???
 * If you call VisitBlock(2, 0) and PrintMap() after that, the stdout would be
 *    ???
 *    12?
 *    01?
 * If you call VisitBlock(0, 1) and PrintMap() after that, the stdout would be
 *    ?X?
 *    12?
 *    01?
 * If the player visits all blocks without mine and call PrintMap() after that, the stdout would be
 *    1@1
 *    122
 *    01@
 * (You may find the global variable game_state useful when implementing this function.)
 *
 * @note Use std::cout to print the game map, especially when you want to try the advanced task!!!
 */
void PrintMap() {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (is_visited[i][j]) {
        // Visited cells
        if (is_mine[i][j]) {
          std::cout << 'X';
        } else {
          std::cout << mine_count[i][j];
        }
      } else if (is_marked[i][j]) {
        // Marked cells
        if (is_mine[i][j]) {
          std::cout << '@';
        } else {
          std::cout << 'X';
        }
      } else {
        // Unvisited cells
        if (game_state == 1) {
          // In victory, show all mines as @
          std::cout << (is_mine[i][j] ? '@' : '?');
        } else {
          std::cout << '?';
        }
      }
    }
    std::cout << std::endl;
  }
}

#endif
