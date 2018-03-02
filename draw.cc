#include "buttons/clear.hh"
#include "buttons/color.hh"
#include "buttons/snapshot.hh"
#include "canvas.hh"
#include "cursor.hh"
#include "screen.hh"

int main ( int argc, char **argv )
{
  Screen screen;

  Cursor cursor( "data/draw.cur" );
  setCursor( cursor );

  Canvas canvas( screen, 1, 0, 15, 9 );

  ColorButton black( screen, 0, 0, canvas, 0, 0, 0 );
  ColorButton violet( screen, 0, 1, canvas, 160, 0, 192 );
  ColorButton blue( screen, 0, 2, canvas, 64, 64, 255 );
  ColorButton green( screen, 0, 3, canvas, 0, 128, 32 );
  ColorButton yellow( screen, 0, 4, canvas, 255, 255, 0 );
  ColorButton orange( screen, 0, 5, canvas, 255, 128, 0 );
  ColorButton red( screen, 0, 6, canvas, 255, 0, 0 );

  SnapShotButton snapShot( screen, 0, 7, canvas );
  ClearButton clear( screen, 0, 8, canvas );

  screen.eventLoop();

  return 0;
}
