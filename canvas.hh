#ifndef CANVAS_HH
#define CANVAS_HH

#include <SDL.h>

#include "screen.hh"


// Canvas
// ------

class Canvas
: public CompatibleSurface,
  public Touchable
{
  typedef ImageSurface PenMask;
  typedef RGBASurface Pen;

  PenMask penMask_;
  Pen pen_;

  void paint ( int x, int y )
  {
    blit( x - pen_.width()/2, y - pen_.height()/2, pen_ );
  }

public:
  Canvas ( Screen &screen, int i, int j, int w, int h )
  : CompatibleSurface( screen, w*120, h*120 ),
    penMask_( "data/pen-small.png" ),
    pen_( penMask_.width(), penMask_.height() )
  {
    pen_.enableAlphaBlending();
    clear();
    setColor( 0, 0, 0 );
    screen.registerTiles( i, j, w, h, surface_, this );
  }

  void clear () { CompatibleSurface::clear( mapRGB( 255, 255, 255 ) ); }

  void setColor ( int r, int g, int b )
  {
    PenMask::Pixels pixelsMask = penMask_.pixels();
    Pen::Pixels pixels = pen_.pixels();
    for( int y = 0; y < pixels.height(); ++y )
      for( int x = 0; x < pixels.width(); ++x )
        pixels( x, y ) = pixels.mapRGBA( r, g, b, 255 - (Uint8)pixelsMask( x, y ) );
  }

  // Touchable

  bool down ( int x, int y ) { paint( x, y ); return true; }
  bool up ( int x, int y ) { paint( x, y ); return true; }

  bool move ( int x, int y, int dx, int dy )
  {
    const int dz = std::max( std::abs( dx ), std::abs( dy ) );
    int ex = dz;
    int ey = dz;
    for( int i = 0; i < dz; ++i )
    {
      paint( x, y );
      if( (ex -= 2*std::abs( dx )) < 0 )
      {
        ex += 2*dz;
        x -= (dx < 0 ? -1 : 1);
      }
      if( (ey -= 2*std::abs( dy )) < 0 )
      {
        ey += 2*dz;
        y -= (dy < 0 ? -1 : 1);
      }
    }
    return true;
  }
};

#endif // #ifndef CANVAS_HH
