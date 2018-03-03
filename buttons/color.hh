#ifndef BUTTONS_COLOR_HH
#define BUTTONS_COLOR_HH

#include <SDL.h>

#include "../canvas.hh"
#include "../screen.hh"
#include "../texture.hh"


// ColorButton
// -----------

class ColorButton
  : public Texture,
    public Touchable
{
  Canvas &canvas_;
  int r_, g_, b_;

public:
  ColorButton ( Screen &screen, int i, int j, Canvas &canvas, int r, int g, int b )
    : Texture( screen, 120, 120, r, g, b ),
      canvas_( canvas ),
      r_( r ), g_( g ), b_( b )
  {
    //clear( mapRGB( r_, g_, b_ ) );
    screen.registerTile( i, j, texture_, this );
  }

  bool down ( int x, int y )
  {
    canvas_.setColor( r_, g_, b_ );
    return false;
  }
};

#endif // #ifndef BUTTONS_COLOR_HH
