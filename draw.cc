#include "buttons/clear.hh"
#include "buttons/color.hh"
#include "buttons/snapshot.hh"
#include "canvas.hh"
#include "cursor.hh"
#include "screen.hh"
#include "snapshots.hh"
#include "webserver.hh"


int main ( int argc, char **argv )
{
  SnapShots snapShots;

  WebServer webServer( 1234 );

  webServer.addPage( "/", [ &snapShots ] ( std::vector< std::string > args, std::ostream &out ) {
      out << "<html>" << std::endl;
      out << "<body>" << std::endl;
      out << "<div width=\"100%\">" << std::endl;
      out << "<h1 width=\"100%\" style=\"background-color: #ff8000\">Today</h1>" << std::endl;
      for( const TimeStamp &timeStamp : snapShots.timeStamps( Date::today() ) )
        out << "<img src=\"/snapshot?" + timeStamp.toQuery() + "\" width=\"33%\"></img>" << std::endl;
      out << "</div>" << std::endl;
      out << "<div width=\"100%\">" << std::endl;
      out << "<h1 width=\"100%\" style=\"background-color: #ff8000\">Yesterday</h1>" << std::endl;
      for( const TimeStamp &timeStamp : snapShots.timeStamps( Date::yesterday() ) )
        out << "<img src=\"/snapshot?" + timeStamp.toQuery() + "\" width=\"33%\"></img>" << std::endl;
      out << "</div>" << std::endl;
      out << "</body>" << std::endl;
      out << "</html>" << std::endl;
      return true;
    } );

  webServer.addPage( "/snapshot", [ &snapShots ] ( std::vector< std::string > args, std::ostream &out ) {
      TimeStamp timeStamp( args[ 0 ], args[ 1 ], args[ 2 ], args[ 3 ], args[ 4 ] );
      if( !snapShots.exists( timeStamp ) )
        return false;
      std::ifstream png( snapShots.toFileName( timeStamp ), std::ios::binary );
      out << png.rdbuf();
      return true;
    }, { "year", "month", "day", "hour", "minute" } );

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

  SnapShotButton snapShot( screen, 0, 7, canvas, snapShots );
  ClearButton clear( screen, 0, 8, canvas );

  screen.eventLoop();

  return 0;
}
