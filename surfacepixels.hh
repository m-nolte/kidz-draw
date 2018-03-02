#ifndef SURFACEPIXELS_HH
#define SURFACEPIXELS_HH

#include <SDL.h>


// SurfacePixels
// -------------

class SurfacePixels
{
  SDL_Surface *surface_;

public:
  class const_reference
  {
  protected:
    const SDL_PixelFormat &format_;
    Uint32 *pixel_;

  public:
    const_reference ( const SDL_PixelFormat &format, void *pixel )
    : format_( format ),
      pixel_( (Uint32 *)pixel )
    {}

    operator Uint32 () const
    {
      const Uint32 mask = (Uint32( 1 ) << format_.BitsPerPixel) - Uint32( 1 );
      return (*pixel_) & mask;
    }
  };

  class reference
  : public const_reference
  {
  public:
    reference ( const SDL_PixelFormat &format, void *pixel )
    : const_reference( format, pixel )
    {}

    const_reference operator= ( Uint32 color )
    {
      const Uint32 mask = ~Uint32( 0 ) >> (32 - format_.BitsPerPixel);
      (*pixel_) = (color & mask) | (*pixel_ & ~mask);
      return *this;
    }
  };

  explicit SurfacePixels ( SDL_Surface *surface )
  : surface_( surface )
  {
    lock();
  }

  SurfacePixels ( const SurfacePixels &other )
  : surface_( other.surface_ )
  {
    lock();
  }

  const SurfacePixels &operator= ( const SurfacePixels &other )
  {
    unlock();
    surface_ = other.surface_;
    lock();
    return *this;
  }

  ~SurfacePixels () { unlock(); }

  const_reference operator() ( int x, int y ) const
  {
    return const_reference( pixelFormat(), (Uint8 *)surface_->pixels + offset( x, y ) );
  }

  reference operator() ( int x, int y )
  {
    return reference( pixelFormat(), (Uint8 *)surface_->pixels + offset( x, y ) );
  }

  const SDL_PixelFormat &pixelFormat () const { return *(surface_->format); }

  int width () const { return surface_->w; }
  int height () const { return surface_->h; }

  Uint32 mapRGB ( int r, int g, int b ) { return SDL_MapRGB( &pixelFormat(), r, g, b ); }
  Uint32 mapRGBA ( int r, int g, int b, int a ) { return SDL_MapRGBA( &pixelFormat(), r, g, b, a ); }

private:
  void lock ()
  {
    if( SDL_MUSTLOCK( surface_ ) )
      SDL_LockSurface( surface_ );
  }

  void unlock ()
  {
    if( SDL_MUSTLOCK( surface_ ) )
      SDL_UnlockSurface( surface_ );
  }

  int offset ( int x, int y ) const
  {
    return y*surface_->pitch + x*pixelFormat().BytesPerPixel;
  }
};

#endif // #ifndef SURFACEPIXELS_HH
