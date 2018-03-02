#ifndef SURFACE_HH
#define SURFACE_HH

#include <SDL.h>
#include <SDL_image.h>

#include "surfacepixels.hh"


// Surface
// -------

class Surface
{
protected:
  SDL_Surface *surface_;

  Surface ()
  : surface_( nullptr )
  {}

  ~Surface () {}

public:
  typedef SurfacePixels Pixels;

  Surface ( const Surface & ) = delete;
  const Surface &operator= ( const Surface & ) = delete;

  const SDL_PixelFormat &pixelFormat () const { return *(surface_->format); }
  Pixels pixels () { return Pixels( surface_ ); } 

  int width () const { return surface_->w; }
  int height () const { return surface_->h; }

  Uint32 mapRGB ( int r, int g, int b ) { return SDL_MapRGB( &pixelFormat(), r, g, b ); }
  Uint32 mapRGBA ( int r, int g, int b, int a ) { return SDL_MapRGBA( &pixelFormat(), r, g, b, a ); }

  void blit ( int x, int y, const Surface &src )
  {
    SDL_Rect rect;
    rect.w = rect.h = 0;
    rect.x = x;
    rect.y = y;
    SDL_BlitSurface( src.surface_, nullptr, surface_, &rect );
  }

  void blit ( int x, int y, const Surface &src, const SDL_Rect &srcRect )
  {
    SDL_Rect rect;
    rect.w = rect.h = 0;
    rect.x = x;
    rect.y = y;
    SDL_BlitSurface( src.surface_, const_cast< SDL_Rect * >( &srcRect ), surface_, &rect );
  }

  void clear ( Uint32 color ) { SDL_FillRect( surface_, nullptr, color ); }

  void enableAlphaBlending ()
  {
    SDL_SetAlpha( surface_, SDL_SRCALPHA, SDL_ALPHA_OPAQUE );
  }

  void disableAlphaBlending ()
  {
    SDL_SetAlpha( surface_, 0, SDL_ALPHA_TRANSPARENT );
  }

  void saveBMP ( const std::string &fileName )
  {
    SDL_SaveBMP( surface_, fileName.c_str() );
  }
};



// RGBASurface
// -----------

class RGBASurface
: public Surface
{
public:
  RGBASurface ( int width, int height )
  {
    surface_ = SDL_CreateRGBSurface( SDL_HWSURFACE, width, height, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 );
  }

  ~RGBASurface () { SDL_FreeSurface( surface_ ); }
};



// CompatibleSurface
// -----------------

class CompatibleSurface
: public Surface
{
public:
  CompatibleSurface ( const Surface &other, int width, int height )
  {
    const SDL_PixelFormat &fmt = other.pixelFormat();
    surface_ = SDL_CreateRGBSurface( SDL_HWSURFACE, width, height, fmt.BitsPerPixel, fmt.Rmask, fmt.Gmask, fmt.Bmask, fmt.Amask );
  }

  ~CompatibleSurface () { SDL_FreeSurface( surface_ ); }
};



// ImageSurface
// ------------

class ImageSurface
: public Surface
{
public:
  explicit ImageSurface ( const std::string &file )
  {
    surface_ = IMG_Load( file.c_str() );
  }

  ~ImageSurface () { SDL_FreeSurface( surface_ ); }
};

#endif // #ifndef SURFACE_HH
