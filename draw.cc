#include "buttons/clear.hh"
#include "buttons/color.hh"
#include "buttons/snapshot.hh"
#include "canvas.hh"
#include "cursor.hh"
#include "screen.hh"
#include "snapshots.hh"
#include "webserver.hh"

extern const char arrow_cursor[];
extern const char empty_cursor[];

extern const std::uint8_t palette_data[];
extern const std::size_t palette_size;


int main ( int argc, char **argv )
{
  Screen screen;
  SnapShots snapShots;

  const bool hasTouchScreen = (SDL_GetNumTouchDevices() > 0);
  std::istringstream cursorIn( hasTouchScreen ? empty_cursor : arrow_cursor );
  Cursor cursor( cursorIn );
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

  WebServer webServer( 1234 );

  webServer.addPage( "/", [ &snapShots ] ( std::vector< std::string > args, std::ostream &out ) {
      out << "<html>" << std::endl;
      out << "<body>" << std::endl;
      out << "<div style=\"width: 100%; background-color: #ff8000;\">" << std::endl;
      out << "<h1 style=\"margin: 5px;\">Today</h1>" << std::endl;
      out << "</div>" << std::endl;
      out << "<div style=\"width: 100%; display: flex; flex-direction: row; flex-wrap: wrap; padding: 0.5%;\">" << std::endl;
      for( const TimeStamp &timeStamp : snapShots.timeStamps( Date::today() ) )
      {
        out << "<div style=\"width: 33%; position: relative; float: left\">" << std::endl;
        out << "<img src=\"/snapshot?" + timeStamp.toQuery() + "\" style=\"width: 100%;\"></img>" << std::endl;
        out << "<a href=\"/edit?" + timeStamp.toQuery() + "\"><img src=\"/palette.png\" style=\"width: 10%; position: absolute; bottom: 8px; right: 8px;\"></img></a>" << std::endl;
        out << "</div>" << std::endl;
      }
      out << "</div>" << std::endl;
      out << "<div style=\"width: 100%; background-color: #ff8000;\">" << std::endl;
      out << "<h1 style=\"margin: 5px;\">Yesterday</h1>" << std::endl;
      out << "</div>" << std::endl;
      out << "<div style=\"width: 100%; display: flex; flex-direction: row; flex-wrap: wrap; padding: 0.5%;\">" << std::endl;
      for( const TimeStamp &timeStamp : snapShots.timeStamps( Date::yesterday() ) )
      {
        out << "<div style=\"width: 33%; position: relative; float: left\">" << std::endl;
        out << "<img src=\"/snapshot?" + timeStamp.toQuery() + "\" style=\"width: 100%;\"></img>" << std::endl;
        out << "<a href=\"/edit?" + timeStamp.toQuery() + "\"><img src=\"/palette.png\" style=\"width: 10%; position: absolute; bottom: 8px; right: 8px;\"></img></a>" << std::endl;
        out << "</div>" << std::endl;
      }
      out << "</div>" << std::endl;
      out << "</body>" << std::endl;
      out << "</html>" << std::endl;
      return true;
    } );

  webServer.addPage( "/palette.png", [] ( std::vector< std::string > args, std::ostream &out ) {
      out.write( reinterpret_cast< const char * >( palette_data ), palette_size );
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

  webServer.addPage( "/edit", [ &screen, &snapShots, &canvas ] ( std::vector< std::string > args, std::ostream &out ) {
      TimeStamp timeStamp( args[ 0 ], args[ 1 ], args[ 2 ], args[ 3 ], args[ 4 ] );
      if( !snapShots.exists( timeStamp ) )
        return false;

      SDL_Event event;
      SDL_memset( &event, 0, sizeof( event ) );
      event.type = screen.lambdaEvent;
      event.user.data1 = new std::function< bool () >( [ &screen, &snapShots, &canvas, timeStamp ] () -> bool {
          //canvas.load( snapShots.toFileName( timeStamp ) );
          canvas.blit( 0, 0, Texture( screen, snapShots.toFileName( timeStamp ) ) );
          return true;
        } );
      SDL_PushEvent( &event );

      out << "<html>" << std::endl;
      out << "<body>" << std::endl;
      out << "<div style=\"width: 100%; background-color: #ff8000;\">" << std::endl;
      out << "<h1 style=\"margin: 5px;\">Image loaded:</h1>" << std::endl;
      out << "</div>" << std::endl;
      out << "<img src=\"/snapshot?" + timeStamp.toQuery() + "\" style=\"width: 100%;\"></img>" << std::endl;
      out << "</body>" << std::endl;
      out << "</html>" << std::endl;
      return true;
    }, { "year", "month", "day", "hour", "minute" } );

  screen.eventLoop();

  return 0;
}
