#ifndef BUTTONS_SNAPSHOT_HH
#define BUTTONS_SNAPSHOT_HH

#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>

#include <SDL.h>

#include "../canvas.hh"
#include "../screen.hh"
#include "../surface.hh"


// SnapShotButton
// --------------

class SnapShotButton
: public ImageSurface,
  public Touchable
{
  Canvas &canvas_;

  std::string fileName ()
  {
    typedef std::chrono::system_clock Clock;
    std::time_t time = Clock::to_time_t( Clock::now() );
    const std::tm *tm = std::localtime( &time );

    std::ostringstream s;
    s << "snapshot-" << std::setfill( '0' )
      << (tm->tm_year + 1900) << "-"
      << std::setw( 2 ) << (tm->tm_mon+1) << "-"
      << std::setw( 2 ) << tm->tm_mday << "-at-"
      << std::setw( 2 ) << tm->tm_hour << "-"
      << std::setw( 2 ) << tm->tm_min << ".bmp";
    return s.str();
  }

public:
  SnapShotButton ( Screen &screen, int i, int j, Canvas &canvas )
  : ImageSurface( "data/camera.png" ),
    canvas_( canvas )
  {
    screen.registerTile( i, j, surface_, this );
  }

  bool down ( int x, int y )
  {
    canvas_.saveBMP( fileName() );
    return false;
  }
};

#endif // #ifndef BUTTONS_CLEAR_HH
