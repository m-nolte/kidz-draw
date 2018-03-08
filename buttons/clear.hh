#ifndef BUTTONS_CLEAR_HH
#define BUTTONS_CLEAR_HH

#include <SDL.h>

#include "../canvas.hh"
#include "../screen.hh"
#include "../texture.hh"


extern const std::uint8_t trash_data[];
extern const std::size_t trash_size;


// ClearButton
// -----------

class ClearButton
  : public Texture,
    public Touchable
{
  Canvas &canvas_;

public:
  ClearButton ( Screen &screen, int i, int j, Canvas &canvas )
    : Texture( screen, trash_data, trash_size ),
      canvas_( canvas )
  {
    screen.registerTile( i, j, texture_, this );
  }

  bool down ( int x, int y )
  {
    canvas_.clear();
    return true;
  }
};

#endif // #ifndef BUTTONS_CLEAR_HH
