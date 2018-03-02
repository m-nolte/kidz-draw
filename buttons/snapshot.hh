#ifndef BUTTONS_SNAPSHOT_HH
#define BUTTONS_SNAPSHOT_HH

#include <iomanip>

#include <SDL.h>

#include "../canvas.hh"
#include "../screen.hh"
#include "../snapshots.hh"
#include "../texture.hh"


// SnapShotButton
// --------------

class SnapShotButton
  : public Texture,
    public Touchable
{
  Canvas &canvas_;
  SnapShots &snapShots_;

public:
  SnapShotButton ( Screen &screen, int i, int j, Canvas &canvas, SnapShots &snapShots )
    : Texture( screen, "data/camera.png" ),
      canvas_( canvas ),
      snapShots_( snapShots )
  {
    screen.registerTile( i, j, texture_, this );
  }

  bool down ( int x, int y )
  {
    try
    {
      canvas_.save( snapShots_.newSnapShot() );
      return false;
    }
    catch( std::exception )
    {
      return false;
    }
  }
};

#endif // #ifndef BUTTONS_CLEAR_HH
