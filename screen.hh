#ifndef SCREEN_HH
#define SCREEN_HH

#include <array>
#include <iostream>

#include <SDL.h>

#include "surface.hh"

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
: public Surface
{
  struct Tile
  {
    SDL_Surface *surface;
    Touchable *touchable;
    SDL_Rect rect;

    Tile ()
    : surface( nullptr ),
      touchable( nullptr )
    {
      rect.y = rect.x = 0;
      rect.h = rect.w = 120;
    }
  };

  std::array< Tile, 16*9 > tiles_;

  const Tile &tile ( int i, int j ) const { return tiles_[ j*16 + i ]; }
  Tile &tile ( int i, int j ) { return tiles_[ j*16 + i ]; }

public:
  Screen ()
  {
    if( SDL_Init( SDL_INIT_VIDEO ) != 0 )
    {
      char *error = SDL_GetError();
      std::cerr << "Unable to initialize SDL: " << error << std::endl;
      exit( 1 );
    }

    const Uint32 videoFlags = SDL_FULLSCREEN | SDL_HWSURFACE;
    surface_ = SDL_SetVideoMode( 1920, 1080, 32, videoFlags );
    //surface_ = SDL_SetVideoMode( 1366, 768, 32, videoFlags );
  }

  ~Screen ()
  {
    SDL_Quit();
  }

  void draw ()
  {
    SDL_Rect rect;
    rect.w = rect.h = 120;
    for( int j = 0; j < 9; ++j )
    {
      rect.y = j*120;
      for( int i = 0; i < 16; ++i )
      {
        rect.x = i*120;
        if( tile( i, j ).surface )
          SDL_BlitSurface( tile( i, j ).surface, &tile( i, j ).rect, surface_, &rect );
      }
    }
    SDL_UpdateRect( surface_, 0, 0, 0, 0 );
  }

  void eventLoop ()
  {
    draw();
    while( true )
    {
      SDL_Event event;
      bool redraw = false;
      while( SDL_PollEvent( &event ) )
      {
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

        case SDL_KEYDOWN:
          if( event.key.keysym.sym == SDLK_ESCAPE )
            return;
        }
      }

      if( redraw )
        draw();
      else
        SDL_Delay( 1 );
    }
  }

  void registerTile ( int i, int j, SDL_Surface *surface, Touchable *touchable, int x = 0, int y = 0 )
  {
    tile( i, j ).surface = surface;
    tile( i, j ).touchable = touchable;
    tile( i, j ).rect.x = x;
    tile( i, j ).rect.y = y;
  }

  void registerTiles ( int i, int j, int w, int h, SDL_Surface *surface, Touchable *touchable, int x = 0, int y = 0 )
  {
    for( int jj = 0; jj < h; ++jj )
      for( int ii = 0; ii < w; ++ii )
        registerTile( i+ii, j+jj, surface, touchable, x+120*ii, y+120*jj );
  }
};

#endif // #ifndef SCREEN_HH
