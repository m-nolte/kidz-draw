#ifndef SCREEN_HH
#define SCREEN_HH

#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

#include <SDL.h>

// Touchable
// ---------

struct Touchable
{
  virtual ~Touchable () {}
  virtual bool down ( int x, int y ) { return false; }
  virtual bool up ( int x, int y ) { return false; }
  virtual bool move ( int x, int y, int dx, int dy ) { return false; }
};



// Screen
// ------

class Screen
{
  friend class Texture;

  struct Tile
  {
    SDL_Texture *texture = nullptr;
    Touchable *touchable = nullptr;
    SDL_Rect rect;

    Tile ()
    {
      rect.y = rect.x = 0;
      rect.h = rect.w = 120;
    }
  };

  SDL_Window *window_ = nullptr;
  SDL_Renderer *renderer_ = nullptr;

  std::array< Tile, 16*9 > tiles_;

public:
  Uint32 lambdaEvent = -1;

private:
  const Tile &tile ( int i, int j ) const { return tiles_[ j*16 + i ]; }
  Tile &tile ( int i, int j ) { return tiles_[ j*16 + i ]; }

public:
  static constexpr int width () { return 1920; }
  static constexpr int height () { return 1080; }

  Screen ()
  {
    if( SDL_Init( SDL_INIT_VIDEO ) != 0 )
    {
      const char *error = SDL_GetError();
      std::cerr << "Unable to initialize SDL: " << error << std::endl;
      exit( 1 );
    }

    //SDL_CreateWindowAndRenderer( 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &window_, &renderer_ );
    window_ = SDL_CreateWindow( "Kidz Draw", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP );
    if( !window_ )
    {
      const char *error = SDL_GetError();
      std::cerr << "Unable to create SDL window: " << error << std::endl;
      SDL_Quit();
      exit( 1 );
    }

    renderer_ = SDL_CreateRenderer( window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE );
    if( !renderer_ )
    {
      const char *error = SDL_GetError();
      std::cerr << "Unable to create SDL renderer: " << error << std::endl;
      SDL_Quit();
      exit( 1 );
    }

    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "linear" );
    SDL_RenderSetLogicalSize( renderer_, width(), height() );

    lambdaEvent = SDL_RegisterEvents( 1 );
  }

  ~Screen ()
  {
    SDL_Quit();
  }

  void draw ()
  {
    SDL_SetRenderDrawColor( renderer_, 0, 0, 0, 255 );
    SDL_RenderClear( renderer_ );

    SDL_Rect rect;
    rect.w = rect.h = 120;
    for( int j = 0; j < 9; ++j )
    {
      rect.y = j*120;
      for( int i = 0; i < 16; ++i )
      {
        rect.x = i*120;
        if( tile( i, j ).texture )
          SDL_RenderCopy( renderer_, tile( i, j ).texture, &tile( i, j ).rect, &rect );
      }
    }

    SDL_RenderPresent( renderer_ );
  }

  void eventLoop ()
  {
    draw();
    std::vector< std::unique_ptr< std::function< bool () > > > lambdaTrashBin;
    while( true )
    {
      SDL_Event event;
      bool redraw = false;
      while( SDL_PollEvent( &event ) )
      {
        if( event.type == lambdaEvent )
        {
          std::function< bool () > *lambda = static_cast< std::function< bool () > * >( event.user.data1 );
          redraw |= (*lambda)();
          lambdaTrashBin.emplace_back( lambda );
        }

        switch( event.type )
        {
        case SDL_QUIT:
          return;

        case SDL_MOUSEBUTTONDOWN:
          if( event.button.button == SDL_BUTTON_LEFT )
          {
            const int i = event.button.x / 120;
            const int j = event.button.y / 120;
            Touchable *touchable = tile( i, j ).touchable;
            if( touchable )
              redraw |= touchable->down( tile( i, j ).rect.x + event.button.x - i*120, tile( i, j ).rect.y + event.button.y - j*120 );
          }
          break;

        case SDL_MOUSEBUTTONUP:
          if( event.button.button == SDL_BUTTON_LEFT )
          {
            const int i = event.button.x / 120;
            const int j = event.button.y / 120;
            Touchable *touchable = tile( i, j ).touchable;
            if( touchable )
              redraw |= touchable->up( tile( i, j ).rect.x + event.button.x - i*120, tile( i, j ).rect.y + event.button.y - j*120 );
          }
          break;

        case SDL_MOUSEMOTION:
          if( event.motion.state & SDL_BUTTON( SDL_BUTTON_LEFT ) )
          {
            const int i = event.motion.x / 120;
            const int j = event.motion.y / 120;
            Touchable *touchable = tile( i, j ).touchable;
            if( touchable )
              redraw |= touchable->move( tile( i, j ).rect.x + event.motion.x - i*120, tile( i, j ).rect.y + event.motion.y - j*120, event.motion.xrel, event.motion.yrel );
          }
          break;

        case SDL_FINGERDOWN:
          {
            const int x = event.tfinger.x * width();
            const int y = event.tfinger.y * height();

            const int i = x / 120;
            const int j = y / 120;
            Touchable *touchable = tile( i, j ).touchable;
            if( touchable )
              redraw |= touchable->down( tile( i, j ).rect.x + x - i*120, tile( i, j ).rect.y + y - j*120 );
          }
          break;

        case SDL_FINGERUP:
          {
            const int x = event.tfinger.x * width();
            const int y = event.tfinger.y * height();

            const int i = x / 120;
            const int j = y / 120;
            Touchable *touchable = tile( i, j ).touchable;
            if( touchable )
              redraw |= touchable->up( tile( i, j ).rect.x + x - i*120, tile( i, j ).rect.y + y - j*120 );
          }
          break;

        case SDL_FINGERMOTION:
          {
            const int x = event.tfinger.x * width();
            const int y = event.tfinger.y * height();
            const int dx = event.tfinger.dx * width();
            const int dy = event.tfinger.dy * height();

            const int i = x / 120;
            const int j = y / 120;
            Touchable *touchable = tile( i, j ).touchable;
            if( touchable )
              redraw |= touchable->move( tile( i, j ).rect.x + x - i*120, tile( i, j ).rect.y + y - j*120, dx, dy );
          }
          break;

        case SDL_KEYDOWN:
          if( event.key.keysym.sym == SDLK_ESCAPE )
            return;
          break;

        case SDL_WINDOWEVENT:
          redraw |= true;
          break;
        }
      }

      if( redraw )
        draw();
      else
        SDL_Delay( 1 );
      lambdaTrashBin.clear();
    }
  }

  void registerTile ( int i, int j, SDL_Texture *texture, Touchable *touchable, int x = 0, int y = 0 )
  {
    tile( i, j ).texture = texture;
    tile( i, j ).touchable = touchable;
    tile( i, j ).rect.x = x;
    tile( i, j ).rect.y = y;
  }

  void registerTiles ( int i, int j, int w, int h, SDL_Texture *texture, Touchable *touchable, int x = 0, int y = 0 )
  {
    for( int jj = 0; jj < h; ++jj )
      for( int ii = 0; ii < w; ++ii )
        registerTile( i+ii, j+jj, texture, touchable, x+120*ii, y+120*jj );
  }
};

#endif // #ifndef SCREEN_HH
