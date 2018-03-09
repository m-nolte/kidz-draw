#include <functional>
#include <map>
#include <sstream>
#include <string>

#include "httpd.hh"


// WebServer
// ---------

class WebServer
{
public:
  using Page = std::function< bool ( std::vector< std::string >, std::ostream & ) >;

private:
  httpd::Daemon daemon_;
  std::map< std::string, std::pair< Page, std::vector< std::string > > > pages_;

public:
  explicit WebServer ( unsigned short port )
    : daemon_( MHD_USE_SELECT_INTERNALLY, port,
               [ this ] ( httpd::Connection connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls ) -> int {
          return this->accept( connection, url, method, version, upload_data, upload_data_size, con_cls );
        } )
  {}

  void addPage ( std::string url, Page page, std::vector< std::string > keywords = {} )
  {
    pages_.emplace( std::move( url ), std::make_pair( std::move( page ), std::move( keywords ) ) );
  }

private:
  int accept ( httpd::Connection connection, std::string url, std::string method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls )
  {
    const auto page = pages_.find( url );
    if( page == pages_.end() )
    {
      httpd::Response response( "<html><body>Resource not found.</body></html>" );
      return connection.queue( httpd::StatusCode::NotFound, response );
    }

    if( method == "GET" )
    {
      std::vector< std::string > args;
      for( const auto &keyword : page->second.second )
      {
        const char *value = connection.lookupValue( MHD_GET_ARGUMENT_KIND, keyword );
        if( !value )
        {
          httpd::Response response( "<html><body>Missing argument: " + keyword + "</body></html>" );
          return connection.queue( httpd::StatusCode::BadRequest, response );
        }
        args.emplace_back( value );
      }

      std::ostringstream out;
      const bool ok = page->second.first( args, out );
      httpd::Response response( out.str() );
      return connection.queue( ok ? httpd::StatusCode::Ok : httpd::StatusCode::BadRequest, response );
    }

    httpd::Response response( "<html><body>Request forbidden.</body></html>" );
    return connection.queue( httpd::StatusCode::Forbidden, response );
  }
};
