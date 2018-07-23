#ifndef TEXTURE_HH
#define TEXTURE_HH

#include <cstdint>

#include <fstream>
#include <sstream>

#include <SDL.h>

#include "png.hh"
#include "screen.hh"

// Texture
// -------

class Texture
{
protected:
  SDL_Renderer *renderer_ = nullptr;
  SDL_Texture *texture_ = nullptr;
  int width_, height_;

public:
  enum class Access
    : int
  {
    Static = SDL_TEXTUREACCESS_STATIC,
    Streaming = SDL_TEXTUREACCESS_STREAMING,
    Target = SDL_TEXTUREACCESS_TARGET
  };

  Texture ( const Screen &screen, const std::string &file )
    : renderer_( screen.renderer_ )
  {
    std::ifstream in( file );
    png::input png_in( in );

    auto info = png_in.read_info();
    width_ = info.image_width();
    height_ = info.image_height();

    Uint32 format = (info.channels() == 4 ? SDL_PIXELFORMAT_ABGR8888 : SDL_PIXELFORMAT_BGR888);
    texture_ = SDL_CreateTexture( renderer_, format, static_cast< int >( Access::Static ), width_, height_ );
    if( info.channels() == 4 )
      SDL_SetTextureBlendMode( texture_, SDL_BLENDMODE_BLEND );

    const std::size_t pitch = info.channels()*width_;
    auto image = png_in.read_image( pitch, height_ );
    SDL_UpdateTexture( texture_, nullptr, image.get(), pitch );
  }

  Texture ( const Screen &screen, const void *data, std::size_t size )
    : renderer_( screen.renderer_ )
  {
    std::istringstream in( std::string( static_cast< const char * >( data ), size ) );
    png::input png_in( in );

    auto info = png_in.read_info();
    width_ = info.image_width();
    height_ = info.image_height();

    Uint32 format = (info.channels() == 4 ? SDL_PIXELFORMAT_ABGR8888 : SDL_PIXELFORMAT_BGR888);
    texture_ = SDL_CreateTexture( renderer_, format, static_cast< int >( Access::Static ), width_, height_ );
    if( info.channels() == 4 )
      SDL_SetTextureBlendMode( texture_, SDL_BLENDMODE_BLEND );

    const std::size_t pitch = info.channels()*width_;
    auto image = png_in.read_image( pitch, height_ );
    SDL_UpdateTexture( texture_, nullptr, image.get(), pitch );
  }

  Texture ( const Screen &screen, int width, int height, Access access = Access::Streaming )
    : renderer_( screen.renderer_ ),
      texture_( SDL_CreateTexture( renderer_, SDL_PIXELFORMAT_ABGR8888, static_cast< int >( access ), width, height ) ),
      width_( width ), height_( height )
  {}

  Texture ( const Screen &screen, int width, int height, Uint8 r, Uint8 g, Uint8 b )
    : renderer_( screen.renderer_ ),
      width_( width ), height_( height )
  {
    SDL_Surface *surface = SDL_CreateRGBSurface( 0, width, height, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 );
    SDL_FillRect( surface, nullptr, SDL_MapRGB( surface->format, r, g, b ) );
    texture_ = SDL_CreateTextureFromSurface( renderer_, surface );
    SDL_FreeSurface( surface );
  }

  Texture ( const Texture & ) = delete;
  Texture ( Texture && ) = delete;

  ~Texture () { SDL_DestroyTexture( texture_ ); }

  Texture &operator= ( const Texture & ) = delete;
  Texture &operator= ( Texture && ) = delete;

  void blit ( int x, int y, const Texture &src )
  {
    SDL_Rect rect;
    rect.w = src.width();
    rect.h = src.height();
    rect.x = x;
    rect.y = y;

    SDL_SetRenderTarget( renderer_, texture_ );
    SDL_RenderCopy( renderer_, src.texture_, nullptr, &rect );
    SDL_SetRenderTarget( renderer_, nullptr );
  }

  void clear ( Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255 )
  {
    SDL_SetRenderTarget( renderer_, texture_ );
    SDL_SetRenderDrawColor( renderer_, r, g, b, a );
    SDL_RenderClear( renderer_ );
    SDL_SetRenderTarget( renderer_, nullptr );
  }

  int width () const { return width_; }
  int height () const { return height_; }

  std::unique_ptr< std::uint8_t[] > pixels () const
  {
    std::unique_ptr< std::uint8_t[] > pixels( new std::uint8_t[ 4*width_*height_ ] );
    SDL_SetRenderTarget( renderer_, texture_ );
    SDL_RenderReadPixels( renderer_, nullptr, SDL_PIXELFORMAT_ABGR8888, pixels.get(), 4*width_ );
    SDL_SetRenderTarget( renderer_, nullptr );
    return pixels;
  }

  void load ( const std::string &file )
  {
    std::ifstream in( file );
    png::input png_in( in );

    auto info = png_in.read_info();
    if( (width_ == info.image_width()) && (height_ == info.image_height()) )
    {
      const std::size_t pitch = info.channels()*width_;
      auto image = png_in.read_image( pitch, height_ );
      SDL_UpdateTexture( texture_, nullptr, image.get(), pitch );
    }
  }

  void save ( std::ostream &out )
  {
    png::output png_out( out );
    png_out.write_info( width_, height_, 8, png::color_type_t::rgb_alpha );
    png_out.write_image( pixels(), 4*width_, height_ );
    png_out.write_end();
  }

  void save ( const std::string &file )
  {
    std::ofstream out( file );
    save( out );
  }

  void saveBMP ( const std::string &fileName )
  {
    SDL_Surface *surface = SDL_CreateRGBSurface( 0, width_, height_, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 );
    SDL_SetRenderTarget( renderer_, texture_ );
    SDL_RenderReadPixels( renderer_, nullptr, SDL_PIXELFORMAT_ABGR8888, surface->pixels, surface->pitch );
    SDL_SetRenderTarget( renderer_, nullptr );
    SDL_SaveBMP( surface, fileName.c_str() );
    SDL_FreeSurface( surface );
  }

  void setColorMod( Uint8 r, Uint8 g, Uint8 b )
  {
    SDL_SetTextureColorMod( texture_, r, g, b );
  }
};

#endif // #ifndef TEXTURE_HH
