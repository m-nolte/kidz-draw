#include <regex>

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



// GalleryResource
// ---------------

class GalleryResource
  : public MicroWebServer::DynamicStringResource
{
  const SnapShots &snapShots_;

public:
  explicit GalleryResource ( const SnapShots &snapShots )
    : MicroWebServer::DynamicStringResource( "text/html" ),
      snapShots_( snapShots )
  {}

  void getContent ( MicroWebServer::Arguments arguments, std::ostream &content ) const override
  {
    using std::to_string;

    content << "<html>" << std::endl;
    content << "<body>" << std::endl;
    content << "<div style=\"width: 100%; background-color: #ff8000;\">" << std::endl;
    content << "<h1 style=\"margin: 5px;\">Today</h1>" << std::endl;
    content << "</div>" << std::endl;
    content << "<div style=\"width: 100%; display: flex; flex-direction: row; flex-wrap: wrap; padding: 0.5%;\">" << std::endl;
    for( const TimeStamp &timeStamp : snapShots_.timeStamps( Date::today() ) )
    {
      content << "<div style=\"width: 33%; position: relative; float: left\">" << std::endl;
      content << "<img src=\"/snapshots/" + to_string( timeStamp ) + ".png\" style=\"width: 100%;\"></img>" << std::endl;
      content << "<a href=\"/edit?" + timeStamp.toQuery() + "\"><img src=\"/palette.png\" style=\"width: 10%; position: absolute; bottom: 8px; right: 8px;\"></img></a>" << std::endl;
      content << "</div>" << std::endl;
    }
    content << "</div>" << std::endl;
    content << "<div style=\"width: 100%; background-color: #ff8000;\">" << std::endl;
    content << "<h1 style=\"margin: 5px;\">Yesterday</h1>" << std::endl;
    content << "</div>" << std::endl;
    content << "<div style=\"width: 100%; display: flex; flex-direction: row; flex-wrap: wrap; padding: 0.5%;\">" << std::endl;
    for( const TimeStamp &timeStamp : snapShots_.timeStamps( Date::yesterday() ) )
    {
      content << "<div style=\"width: 33%; position: relative; float: left\">" << std::endl;
      content << "<img src=\"/snapshots/" + to_string( timeStamp ) + ".png\" style=\"width: 100%;\"></img>" << std::endl;
      content << "<a href=\"/edit?" + timeStamp.toQuery() + "\"><img src=\"/palette.png\" style=\"width: 10%; position: absolute; bottom: 8px; right: 8px;\"></img></a>" << std::endl;
      content << "</div>" << std::endl;
    }
    content << "</div>" << std::endl;
    content << "</body>" << std::endl;
    content << "</html>" << std::endl;
  }
};


// SnapShotsResource
// -----------------

class SnapShotsResource
  : public MicroWebServer::Resource
{
  const SnapShots &snapShots_;
  std::regex pattern_;

public:
  explicit SnapShotsResource ( const SnapShots &snapShots )
    : snapShots_( snapShots ),
      pattern_( "/([0-9]{4})-([0-9]{2})-([0-9]{2})-at-([0-9]{2})-([0-9]{2})[.]png" )
  {}

  std::shared_ptr< Resource > operator[] ( std::string url ) override
  {
    std::smatch subMatch;
    if( !std::regex_match( url, subMatch, pattern_ ) )
      return nullptr;

    const TimeStamp timeStamp( subMatch[ 1 ].str(), subMatch[ 2 ].str(), subMatch[ 3 ].str(), subMatch[ 4 ].str(), subMatch[ 5 ].str() );
    if( !snapShots_.exists( timeStamp ) )
      return nullptr;

    return std::make_shared< MicroWebServer::FileResource >( snapShots_.toFileName( timeStamp ), "image/png" );
  }
};



// EditResource
// ------------

class EditResource
  : public MicroWebServer::Resource
{
  const SnapShots &snapShots_;
  Screen &screen_;
  Canvas &canvas_;

public:
  explicit EditResource ( const SnapShots &snapShots, Screen &screen, Canvas &canvas )
    : snapShots_( snapShots ), screen_( screen ), canvas_( canvas )
  {}


  std::unique_ptr< httpd::RequestHandler > getGetHandler ( httpd::Connection connection ) const override
  {
    using std::to_string;

    MicroWebServer::Arguments arguments( connection );
    const TimeStamp timeStamp( arguments[ "year" ], arguments[ "month" ], arguments[ "day" ], arguments[ "hour" ], arguments[ "minute" ] );
    if( !snapShots_.exists( timeStamp ) )
      return httpd::makeNotFoundRequestHandler();

    SDL_Event event;
    SDL_memset( &event, 0, sizeof( event ) );
    event.type = screen_.lambdaEvent;
    event.user.data1 = new std::function< bool () >( [ this, timeStamp ] () -> bool {
        //canvas.load( snapShots.toFileName( timeStamp ) );
        canvas_.blit( 0, 0, Texture( screen_, snapShots_.toFileName( timeStamp ) ) );
        return true;
      } );
    SDL_PushEvent( &event );

    std::ostringstream content;
    content << "<html>" << std::endl;
    content << "<body>" << std::endl;
    content << "<div style=\"width: 100%; background-color: #ff8000;\">" << std::endl;
    content << "<h1 style=\"margin: 5px;\">Image loaded:</h1>" << std::endl;
    content << "</div>" << std::endl;
    content << "<img src=\"/snapshots/" + to_string( timeStamp ) + ".png\" style=\"width: 100%;\"></img>" << std::endl;
    content << "</body>" << std::endl;
    content << "</html>" << std::endl;
    return httpd::makeContentRequestHandler( "text/html", content.str() );
  }
};



// main
// ----

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

  auto webRoot = std::make_shared< MicroWebServer::MapResource >();

  webRoot->add( "/", std::make_shared< MicroWebServer::RedirectResource >( "gallery" ) );
  webRoot->add( "/gallery", std::make_shared< GalleryResource >( snapShots ) );
  webRoot->add( "/snapshots", std::make_shared< SnapShotsResource >( snapShots ) );
  webRoot->add( "/edit", std::make_shared< EditResource >( snapShots, screen, canvas ) );
  webRoot->add( "/palette.png", std::make_shared< MicroWebServer::StaticDataResource >( palette_data, palette_size, "image/png" ) );

  MicroWebServer::WebServer webServer( 1234, webRoot );

  screen.eventLoop();

  return 0;
}
