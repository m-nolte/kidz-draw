#ifndef CANVAS_HH
#define CANVAS_HH

#include <SDL.h>

#include "screen.hh"
#include "texture.hh"


extern const std::uint8_t pen_small_data[];
extern const std::size_t pen_small_size;


// Canvas
// ------

class Canvas
  : public Texture,
    public Touchable
{
  Texture pen_;

  void paint ( int x, int y )
  {
    blit( x - pen_.width()/2, y - pen_.height()/2, pen_ );
  }

public:
  Canvas ( Screen &screen, int i, int j, int w, int h )
    : Texture( screen, w*120, h*120, Texture::Access::Target ),
      pen_( screen, pen_small_data, pen_small_size )
  {
    clear();
    setColor( 0, 0, 0 );
    screen.registerTiles( i, j, w, h, texture_, this );
  }

  void clear () { Texture::clear( 255, 255, 255 ); }

  void setColor ( int r, int g, int b )
  {
    pen_.setColorMod( r, g, b );
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
