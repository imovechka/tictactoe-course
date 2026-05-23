#pragma once

#include "core/game.hpp"
#include <vector>

namespace ttt::my_player {

using game::Event;
using game::IPlayer;
using game::Point;
using game::Sign;
using game::State;

class MyPlayer : public IPlayer {
  Sign m_sign = Sign::NONE;
  const char *m_name;
  int m_depth;            // D — глубина поиска
  int m_base;             // base — коэффициент эвристики
  bool m_last_move_for_o; // флаг последнего хода для O

public:
  MyPlayer(const char *name, int depth = 2, int base = 3)
      : m_sign(Sign::NONE), m_name(name),
        m_depth(depth), m_base(base),
        m_last_move_for_o(false) {}

  void set_sign(Sign sign) override;
  Point make_move(const State &state) override;
  const char *get_name() const override;

  // Публичные методы для юнит-тестов
  bool check_win(const State &state, int x, int y, Sign sign) const;
  int evaluate(const State &state) const;

  // Клетки-кандидаты: свободные клетки в радиусе radius от занятых
  std::vector<Point> candidates(const State &state, int radius = 2) const;

private:
  int minimax(State state, int depth, int alpha, int beta, bool maximizing) const;
  static int ipow(int base, int exp);
  void scan_line(const State &state,
                 int x0, int y0, int dx, int dy, int len,
                 int &score_o, int &score_x) const;
};

}; // namespace ttt::my_player
