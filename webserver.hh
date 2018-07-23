#include <functional>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

#include "httpd.hh"

namespace MicroWebServer
{

  // Imported Types
  // --------------

  using httpd::Buffer;
  using httpd::Connection;
  using httpd::RequestHandler;
  using httpd::StatusCode;



  // Arguments
  // ---------

  class Arguments
  {
    Connection connection_;

  public:
    Arguments ( Connection connection )
      : connection_( connection )
    {}

    std::string operator[] ( const std::string &name ) const
    {
      const char *value = connection_.lookupValue( MHD_GET_ARGUMENT_KIND, name );
      if( value )
        return std::string( value );
      else
        throw std::out_of_range( "No such Argument" );
    }
  };



  // Resource
  // --------

  class Resource
    : protected std::enable_shared_from_this< Resource >
  {
  public:
    virtual ~Resource () = default;

    virtual std::shared_ptr< Resource > operator[] ( std::string url ) { return (url == "/" ? shared_from_this() : nullptr); }

    virtual std::unique_ptr< RequestHandler > getGetHandler ( Connection connection ) const { return httpd::makeMethodNotAllowedRequestHandler(); }
    virtual std::unique_ptr< RequestHandler > getHeadHandler ( Connection connection ) const { return httpd::makeMethodNotAllowedRequestHandler(); }

    virtual std::unique_ptr< RequestHandler > getPostHandler ( Connection connection ) { return httpd::makeMethodNotAllowedRequestHandler(); }
    virtual std::unique_ptr< RequestHandler > getPutHandler ( Connection connection ) { return httpd::makeMethodNotAllowedRequestHandler(); }
    virtual std::unique_ptr< RequestHandler > getPatchHandler ( Connection connection ) { return httpd::makeMethodNotAllowedRequestHandler(); }

    virtual std::unique_ptr< RequestHandler > getDeleteHandler ( Connection connection ) { return httpd::makeMethodNotAllowedRequestHandler(); }
  };



  // MapResource
  // -----------

  class MapResource
    : public Resource
  {
    std::map< std::string, std::shared_ptr< Resource > > resources_;

  public:
    std::shared_ptr< Resource > operator[] ( std::string url ) override
    {
      const auto pos = resources_.find( url );
      if( pos != resources_.end() )
        return pos->second;

      for( std::size_t count = url.find_last_of( '/' ); count > 0; count = url.find_last_of( '/', count-1 ) )
      {
        const auto pos = resources_.find( url.substr( 0, count ) );
        if( pos != resources_.end() )
          return (*pos->second)[ url.substr( count ) ];
      }

      return nullptr;
    }

    std::unique_ptr< RequestHandler > getGetHandler ( Connection connection ) const override { return root()->getGetHandler( connection ); }
    std::unique_ptr< RequestHandler > getHeadHandler ( Connection connection ) const override { return root()->getHeadHandler( connection ); }

    std::unique_ptr< RequestHandler > getPostHandler ( Connection connection ) override { return root()->getPostHandler( connection ); }
    std::unique_ptr< RequestHandler > getPutHandler ( Connection connection ) override { return root()->getPutHandler( connection ); }
    std::unique_ptr< RequestHandler > getPatchHandler ( Connection connection ) override { return root()->getPatchHandler( connection ); }

    void add ( std::string url, std::shared_ptr< Resource > resource )
    {
      resources_.emplace( std::move( url ), std::move( resource ) );
    }

    std::shared_ptr< const Resource > root () const
    {
      const auto pos = resources_.find( "/" );
      return (pos != resources_.end() ? pos->second : std::make_shared< Resource >());
    }

    std::shared_ptr< Resource > root ()
    {
      const auto pos = resources_.find( "/" );
      return (pos != resources_.end() ? pos->second : std::make_shared< Resource >());
    }
  };



  // StaticDataResource
  // ------------------

  class StaticDataResource
    : public Resource
  {
    const void *address_;
    size_t size_;
    std::string contentType_;

  public:
    explicit StaticDataResource ( const void *address, size_t size )
      : address_( address ), size_( size ), contentType_( "application/octet-stream" )
    {}

    explicit StaticDataResource ( const void *address, size_t size, std::string contentType )
      : address_( address ), size_( size ), contentType_( std::move( contentType ) )
    {}

    std::unique_ptr< RequestHandler > getGetHandler ( Connection connection ) const override
    {
      return httpd::makeContentRequestHandler( contentType_, address_, size_, MHD_RESPMEM_PERSISTENT );
    }

    std::unique_ptr< RequestHandler > getHeadHandler ( Connection connection ) const override
    {
      return httpd::makeContentRequestHandler( contentType_ );
    }
  };



  // FileResource
  // ------------

  class FileResource
    : public Resource
  {
    std::string fileName_;
    std::string contentType_;

  public:
    explicit FileResource ( std::string fileName )
      : fileName_( std::move( fileName ) ), contentType_( "application/octet-stream" )
    {}

    FileResource ( std::string fileName, std::string contentType )
      : fileName_( std::move( fileName ) ), contentType_( std::move( contentType ) )
    {}

    std::unique_ptr< RequestHandler > getGetHandler ( Connection connection ) const override
    {
      httpd::File file( fileName_, O_RDONLY );
      if( file )
        return httpd::makeContentRequestHandler( contentType_, std::move( file ) );
      else
        return httpd::makeNotFoundRequestHandler();
    }

    std::unique_ptr< RequestHandler > getHeadHandler ( Connection connection ) const override
    {
      httpd::File file( fileName_, O_RDONLY );
      if( file )
        return httpd::makeContentRequestHandler( contentType_ );
      else
        return httpd::makeNotFoundRequestHandler();
    }
  };



  // VirtualDirectoryResource
  // ------------------------

  class VirtualDirectoryResource
    : public Resource
  {
    std::function< std::shared_ptr< Resource > ( std::string ) > makeResource_;

  public:
    explicit VirtualDirectoryResource ( std::function< std::shared_ptr< Resource > ( std::string ) > makeResource )
      : makeResource_( std::move( makeResource ) )
    {}

    std::shared_ptr< Resource > operator[] ( std::string url ) override
    {
      return makeResource_( std::move( url ) );
    }
  };


  // DirectoryResource
  // -----------------

  class DirectoryResource
    : public VirtualDirectoryResource
  {
  public:
    explicit DirectoryResource ( std::string directory )
      : VirtualDirectoryResource( [ directory ] ( std::string url ) {
            return std::make_shared< FileResource >( directory + url );
          } )
    {}
  };



  // StringResource
  // --------------

  class StringResource
    : public Resource
  {
    std::string content_;
    std::string contentType_;

  public:
    explicit StringResource ( std::string content )
      : content_( std::move( content ) ), contentType_( "text/plain" )
    {}

    StringResource ( std::string content, std::string contentType )
      : content_( std::move( content ) ), contentType_( std::move( contentType ) )
    {}

    std::unique_ptr< RequestHandler > getGetHandler ( Connection connection ) const override
    {
      return httpd::makeContentRequestHandler( contentType_, content_ );
    }

    std::unique_ptr< RequestHandler > getHeadHandler ( Connection connection ) const override
    {
      return httpd::makeContentRequestHandler( contentType_ );
    }
  };



  // DynamicStringResource
  // ---------------------

  class DynamicStringResource
    : public Resource
  {
    std::string contentType_;

  protected:
    DynamicStringResource ()
      : contentType_( "text/plain" )
    {}

    explicit DynamicStringResource ( std::string contentType )
      : contentType_( std::move( contentType ) )
    {}

  public:
    virtual void getContent ( Arguments arguments, std::ostream &content ) const = 0;

    std::unique_ptr< RequestHandler > getGetHandler ( Connection connection ) const override
    {
      std::ostringstream content;
      getContent( Arguments( connection ), content );
      return httpd::makeContentRequestHandler( contentType_, content.str() );
    }

    std::unique_ptr< RequestHandler > getHeadHandler ( Connection connection ) const override
    {
      return httpd::makeContentRequestHandler( contentType_ );
    }
  };



  // RedirectResource
  // ----------------

  class RedirectResource
    : public Resource
  {
    std::string url_;

  public:
    explicit RedirectResource ( std::string url )
      : url_( std::move( url ) )
    {}

    std::unique_ptr< RequestHandler > getGetHandler ( Connection connection ) const override { return httpd::makeRedirectRequestHandler( url_ ); }
    std::unique_ptr< RequestHandler > getHeadHandler ( Connection connection ) const override { return httpd::makeRedirectRequestHandler( url_ ); }

    std::unique_ptr< RequestHandler > getPostHandler ( Connection connection ) override { return httpd::makeRedirectRequestHandler( url_ ); }
    std::unique_ptr< RequestHandler > getPutHandler ( Connection connection ) override { return httpd::makeRedirectRequestHandler( url_ ); }
    std::unique_ptr< RequestHandler > getPatchHandler ( Connection connection ) override { return httpd::makeRedirectRequestHandler( url_ ); }

    std::unique_ptr< RequestHandler > getDeleteHandler ( Connection connection ) override { return httpd::makeRedirectRequestHandler( url_ ); }
  };



  // WebServer
  // ---------

  class WebServer
    : private httpd::Daemon
  {
    std::shared_ptr< Resource > root_;

  public:
    WebServer ( unsigned short port, std::shared_ptr< Resource > root )
      : httpd::Daemon( MHD_USE_SELECT_INTERNALLY, port ), root_( std::move( root ) )
    {}

  private:
    std::unique_ptr< RequestHandler > getRequestHandler ( Connection connection, const char *url, const char *method, const char *version ) override
    {
      // Note: url is a C string, so even if the string is empty url[ 0 ] is a valid character (namely 0)
      if( url[ 0 ] != '/' )
        return httpd::makeResponseRequestHandler( StatusCode::BadRequest );

      if( !root_ )
        return httpd::makeNotFoundRequestHandler();;

      // Note: Here, url is implicitly converted to std::string
      std::shared_ptr< Resource > resource = (*root_)[ url ];
      if( !resource )
        return httpd::makeNotFoundRequestHandler();

      if( std::strcmp( method, MHD_HTTP_METHOD_GET ) == 0 )
        return resource->getGetHandler( connection );
      else if( std::strcmp( method, MHD_HTTP_METHOD_HEAD ) == 0 )
        return resource->getHeadHandler( connection );
      else if( std::strcmp( method, MHD_HTTP_METHOD_POST ) == 0 )
        return resource->getPostHandler( connection );
      else if( std::strcmp( method, MHD_HTTP_METHOD_PUT ) == 0 )
        return resource->getPutHandler( connection );
      else if( std::strcmp( method, MHD_HTTP_METHOD_PATCH ) == 0 )
        return resource->getPatchHandler( connection );
      else if( std::strcmp( method, MHD_HTTP_METHOD_DELETE ) == 0 )
        return resource->getDeleteHandler( connection );
      else
        return httpd::makeMethodNotAllowedRequestHandler();
    }
  };

}
