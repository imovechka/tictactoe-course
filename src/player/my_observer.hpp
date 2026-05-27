#pragma once
#include <iostream>
#include <ostream>

#include "core/game.hpp"

namespace ttt::my_player {

using game::Event;
using game::IObserver;
using game::State;

class ConsoleWriter : public IObserver {
public:
  static void print_game_state(const State &state);
  void handle_event(const State &game, const Event &event) override;
};
class OstreamWriter : public IObserver {
  std::ostream& out_;  // ссылка на поток вывода

public:
  OstreamWriter() : out_(std::cout) {}              // консоль
  OstreamWriter(std::ostream& os) : out_(os) {}     // можно передать файл
	// Метод печати доски
  void print_game_state(const State &state);
	// Обработчик событий 
  void handle_event(const State &game, const Event &event) override;
};

}; // namespace ttt::my_player
