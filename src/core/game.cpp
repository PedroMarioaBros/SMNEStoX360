#include "smb360/game.hpp"
namespace smb360 {
Game::Game(){ reset(); }
void Game::reset(){ frame_=0; pixels_.fill(0); }
void Game::tick(const InputState&){ ++frame_; }
}
