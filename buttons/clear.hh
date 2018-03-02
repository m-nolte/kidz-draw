#ifndef BUTTONS_CLEAR_HH
#define BUTTONS_CLEAR_HH

#include <SDL.h>

#include "../canvas.hh"
#include "../screen.hh"
#include "../surface.hh"


// ClearButton
// -----------

class ClearButton
: public ImageSurface,
  public Touchable
{
  Canvas &canvas_;

public:
  ClearButton ( Screen &screen, int i, int j, Canvas &canvas )
  : ImageSurface( "data/trash.png" ),
    canvas_( canvas )
  {
    screen.registerTile( i, j, surface_, this );
  }

  bool down ( int x, int y )
  {
    canvas_.clear();
    return true;
  }
};

#endif // #ifndef BUTTONS_CLEAR_HH
