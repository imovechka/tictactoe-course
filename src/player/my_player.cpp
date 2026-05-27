#include "my_player.hpp"
#include <climits>
#include <vector>
#include <algorithm>

namespace ttt::my_player {

void MyPlayer::set_sign(Sign sign) { m_sign = sign; }
const char *MyPlayer::get_name() const { return m_name; }

int MyPlayer::ipow(int base, int exp) {
  int result = 1;
  for (int i = 0; i < exp; ++i) result *= base;
  return result;
}

//  Клетки-кандидаты: свободные соседи занятых клеток 

std::vector<Point> MyPlayer::candidates(const State &state, int radius) const {
  const int cols = state.get_opts().cols;
  const int rows = state.get_opts().rows;

  std::vector<bool> visited(cols * rows, false);
  std::vector<Point> result;

  for (int x = 0; x < cols; ++x) {
    for (int y = 0; y < rows; ++y) {
      Sign v = state.get_value(x, y);
      if (v != Sign::X && v != Sign::O) continue;
      for (int dx = -radius; dx <= radius; ++dx) {
        for (int dy = -radius; dy <= radius; ++dy) {
          int nx = x + dx, ny = y + dy;
          if (nx < 0 || nx >= cols || ny < 0 || ny >= rows) continue;
          if (state.get_value(nx, ny) != Sign::NONE) continue;
          int idx = ny * cols + nx;
          if (!visited[idx]) {
            visited[idx] = true;
            result.push_back({nx, ny});
          }
        }
      }
    }
  }
  return result;
}

//  check_win 

bool MyPlayer::check_win(const State &state, int x, int y, Sign sign) const {
  if (state.get_value(x, y) != Sign::NONE) return false;

  const int L    = state.get_opts().win_len;
  const int cols = state.get_opts().cols;
  const int rows = state.get_opts().rows;
  const int dirs[4][2] = {{1,0},{0,1},{1,1},{1,-1}};

  for (auto &d : dirs) {
    int count = 1;
    for (int s = 1; s < L; ++s) {
      int nx = x+d[0]*s, ny = y+d[1]*s;
      if (nx<0||nx>=cols||ny<0||ny>=rows) break;
      if (state.get_value(nx,ny) != sign) break;
      ++count;
    }
    for (int s = 1; s < L; ++s) {
      int nx = x-d[0]*s, ny = y-d[1]*s;
      if (nx<0||nx>=cols||ny<0||ny>=rows) break;
      if (state.get_value(nx,ny) != sign) break;
      ++count;
    }
    if (count >= L) return true;
  }
  return false;
}

//  scan_line: W = base^k * M 

void MyPlayer::scan_line(const State &state,
                         int x0, int y0, int dx, int dy, int len,
                         int &score_my, int &score_opp) const {
  const int L    = state.get_opts().win_len;
  const int cols = state.get_opts().cols;
  const int rows = state.get_opts().rows;
  Sign opp = (m_sign == Sign::O) ? Sign::X : Sign::O;

  int i = 0;
  while (i < len) {
    int cx = x0+dx*i, cy = y0+dy*i;
    Sign cur = state.get_value(cx, cy);

    if (cur == Sign::NONE || cur == Sign::WALL) { ++i; continue; }

    Sign chain_sign = cur;
    int start = i, k = 0;
    while (i < len && state.get_value(x0+dx*i, y0+dy*i) == chain_sign) {
      ++k; ++i;
    }
    if (k >= L) continue;

    int open_ends = 0;
    if (start > 0) {
      int lx = x0+dx*(start-1), ly = y0+dy*(start-1);
      if (lx>=0&&lx<cols&&ly>=0&&ly<rows &&
          state.get_value(lx,ly)==Sign::NONE) ++open_ends;
    }
    {
      int rx = x0+dx*i, ry = y0+dy*i;
      if (i<len && rx>=0&&rx<cols&&ry>=0&&ry<rows &&
          state.get_value(rx,ry)==Sign::NONE) ++open_ends;
    }

    int w = ipow(m_base, k) * open_ends;

    // Правильно: my_sign vs opp — не зависит от O/X
    if (chain_sign == m_sign) score_my   += w;
    else                      score_opp  += w;
  }
}

//  evaluate: Score = score_my - score_opp (относительно m_sign) 

int MyPlayer::evaluate(const State &state) const {
  const int cols = state.get_opts().cols;
  const int rows = state.get_opts().rows;
  int sm = 0, so = 0;

  for (int y = 0; y < rows; ++y)
    scan_line(state,0,y,1,0,cols,sm,so);
  for (int x = 0; x < cols; ++x)
    scan_line(state,x,0,0,1,rows,sm,so);
  for (int x = 0; x < cols; ++x)
    scan_line(state,x,0,1,1,std::min(cols-x,rows),sm,so);
  for (int y = 1; y < rows; ++y)
    scan_line(state,0,y,1,1,std::min(cols,rows-y),sm,so);
  for (int x = 0; x < cols; ++x)
    scan_line(state,x,rows-1,1,-1,std::min(cols-x,rows),sm,so);
  for (int y = 0; y < rows-1; ++y)
    scan_line(state,0,y,1,-1,std::min(cols,y+1),sm,so);

  return sm - so;
}

//  minimax 

int MyPlayer::minimax(State state, int depth, int alpha, int beta,
                      bool maximizing) const {
  Sign opp = (m_sign == Sign::O) ? Sign::X : Sign::O;

  if (state.get_winner() == m_sign) return  1000000 + depth;
  if (state.get_winner() == opp)   return -1000000 - depth;
  if (depth == 0)                  return evaluate(state);

  Sign cur_sign = maximizing ? m_sign : opp;

auto cands = candidates(state, 1);
if (cands.empty()) return evaluate(state);

 

  if (maximizing) {
    int best = INT_MIN;
    for (const Point &p : cands) {
      State next = state;
      next.process_move(cur_sign, p.x, p.y);
      int score = minimax(next, depth-1, alpha, beta, false);
      if (score > best) best = score;
      if (best > alpha) alpha = best;
      if (beta <= alpha) break;
    }
    return best;
  } else {
    int best = INT_MAX;
    for (const Point &p : cands) {
      State next = state;
      next.process_move(cur_sign, p.x, p.y);
      int score = minimax(next, depth-1, alpha, beta, true);
      if (score < best) best = score;
      if (best < beta)  beta = best;
      if (beta <= alpha) break;
    }
    return best;
  }
}

//  make_move 

Point MyPlayer::make_move(const State &state) {
  const int cols = state.get_opts().cols;
  const int rows = state.get_opts().rows;
  Sign opp = (m_sign == Sign::O) ? Sign::X : Sign::O;

  // Шаг 1: список свободных клеток
  std::vector<Point> free_cells;
  for (int x = 0; x < cols; ++x)
    for (int y = 0; y < rows; ++y)
      if (state.get_value(x, y) == Sign::NONE)
        free_cells.push_back({x, y});

  if (free_cells.empty()) return {-1, -1};

  Point best_move = free_cells[0];

  // Шаг 2: немедленная победа
  for (const Point &p : free_cells)
    if (check_win(state, p.x, p.y, m_sign)) { best_move = p; goto done; }

  // Шаг 3: блокировка победы противника
  for (const Point &p : free_cells)
    if (check_win(state, p.x, p.y, opp)) { best_move = p; goto done; }

  // Шаг 4: флаг last_move_for_O
  if (m_last_move_for_o) {
    for (const Point &p : free_cells)
      if (check_win(state, p.x, p.y, m_sign)) { best_move = p; goto done; }
    m_last_move_for_o = false;
  }

  // Шаг 5: стратегический выбор через minimax по кандидатам
  {
    auto cands = candidates(state, 2);
    if (cands.empty()) cands = free_cells;

    int best_score = INT_MIN;
    for (const Point &p : cands) {
      State next = state;
      next.process_move(m_sign, p.x, p.y);
      int score = minimax(next, m_depth-1, INT_MIN, INT_MAX, false);
      if (score > best_score) {
        best_score = score;
        best_move  = p;
      }
    }
  }

done:
  m_last_move_for_o = false;
  return best_move;
}

}; // namespace ttt::my_player
