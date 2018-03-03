#ifndef BUTTONS_CLEAR_HH
#define BUTTONS_CLEAR_HH

#include <SDL.h>

#include "../canvas.hh"
#include "../screen.hh"
#include "../texture.hh"


// ClearButton
// -----------

class ClearButton
  : public Texture,
    public Touchable
{
  Canvas &canvas_;

public:
  ClearButton ( Screen &screen, int i, int j, Canvas &canvas )
    : Texture( screen, "data/trash.png" ),
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
