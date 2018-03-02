#ifndef BUTTONS_COLOR_HH
#define BUTTONS_COLOR_HH

#include <SDL.h>

#include "../canvas.hh"
#include "../screen.hh"
#include "../surface.hh"


// ColorButton
// -----------

class ColorButton
: public CompatibleSurface,
  public Touchable
{
  Canvas &canvas_;
  int r_, g_, b_;

public:
  ColorButton ( Screen &screen, int i, int j, Canvas &canvas, int r, int g, int b )
  : CompatibleSurface( screen, 120, 120 ),
    canvas_( canvas ),
    r_( r ), g_( g ), b_( b )
  {
    clear( mapRGB( r_, g_, b_ ) );
    screen.registerTile( i, j, surface_, this );
  }

  bool down ( int x, int y )
  {
    canvas_.setColor( r_, g_, b_ );
    return false;
  }
};

#endif // #ifndef BUTTONS_COLOR_HH
