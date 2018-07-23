#ifndef HTTPD_HH
#define HTTPD_HH

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include <fcntl.h>
#include <microhttpd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace httpd
{

  // Internal Forward Declarations
  // -----------------------------

  class Connection;
  class Daemon;



  // StatusCode
  // ----------

  enum class StatusCode
    : unsigned int
  {
    Continue = MHD_HTTP_CONTINUE,
    SwitchingProtocols = MHD_HTTP_SWITCHING_PROTOCOLS,
    Processing = MHD_HTTP_PROCESSING,

    Ok = MHD_HTTP_OK,
    Created = MHD_HTTP_CREATED,
    Accepted = MHD_HTTP_ACCEPTED,
    NonAuthoritativeInformation = MHD_HTTP_NON_AUTHORITATIVE_INFORMATION,
    NoContent = MHD_HTTP_NO_CONTENT,
    ResetContent = MHD_HTTP_RESET_CONTENT,
    PartialContent = MHD_HTTP_PARTIAL_CONTENT,
    MultiStatus = MHD_HTTP_MULTI_STATUS,
#if MHD_VERSION >= 0x00095800
    AlreadyReported = MHD_HTTP_ALREADY_REPORTED,
#endif // #if MHD_VERSION >= 0x00095800

    MultipleChoices = MHD_HTTP_MULTIPLE_CHOICES,
    MovedPermanently = MHD_HTTP_MOVED_PERMANENTLY,
    Found = MHD_HTTP_FOUND,
    SeeOther = MHD_HTTP_SEE_OTHER,
    NotModified = MHD_HTTP_NOT_MODIFIED,
    UseProxy = MHD_HTTP_USE_PROXY,
    SwitchProxy = MHD_HTTP_SWITCH_PROXY,
    TemporaryRedirect = MHD_HTTP_TEMPORARY_REDIRECT,
    PermanentRedicect = MHD_HTTP_PERMANENT_REDIRECT,

    BadRequest = MHD_HTTP_BAD_REQUEST,
    Unauthorized = MHD_HTTP_UNAUTHORIZED,
    PaymentRequired = MHD_HTTP_PAYMENT_REQUIRED,
    Forbidden = MHD_HTTP_FORBIDDEN,
    NotFound = MHD_HTTP_NOT_FOUND,
    MethodNotAllowed = MHD_HTTP_METHOD_NOT_ALLOWED,
    NotAcceptable = MHD_HTTP_NOT_ACCEPTABLE,
    ProxyAuthenticationRequired = MHD_HTTP_PROXY_AUTHENTICATION_REQUIRED,
    RequestTimeOut = MHD_HTTP_REQUEST_TIMEOUT,
    Conflict = MHD_HTTP_CONFLICT,
    Gone = MHD_HTTP_GONE,
    LengthRequired = MHD_HTTP_LENGTH_REQUIRED,
    PreconditionFailed = MHD_HTTP_PRECONDITION_FAILED,
#if MHD_VERSION >= 0x00095500
    PayloadTooLarge = MHD_HTTP_PAYLOAD_TOO_LARGE,
    URITooLong = MHD_HTTP_URI_TOO_LONG,
#else // #if MHD_VERSION >= 0x00095500
    PayloadTooLarge = MHD_HTTP_REQUEST_ENTITY_TOO_LARGE,
    URITooLong = MHD_HTTP_REQUEST_URI_TOO_LONG,
#endif // #else // #if MHD_VERSION >= 0x00095500
    UnsupportedMediaType = MHD_HTTP_UNSUPPORTED_MEDIA_TYPE,
#if MHD_VERSION >= 0x00095500
    RangeNotSatisfiable = MHD_HTTP_RANGE_NOT_SATISFIABLE,
#else // #if MHD_VERSION >= 0x00095500
    RangeNotSatisfiable = MHD_HTTP_REQUESTED_RANGE_NOT_SATISFIABLE,
#endif // #else // #if MHD_VERSION >= 0x00095500
    ExprectationFailed = MHD_HTTP_EXPECTATION_FAILED,
#if MHD_VERSION >= 0x00095800
    MisdirectedRequest = MHD_HTTP_MISDIRECTED_REQUEST,
#endif // #if MHD_VERSION >= 0x00095800
    UnprocessableEntity = MHD_HTTP_UNPROCESSABLE_ENTITY,
    Locked = MHD_HTTP_LOCKED,
    FailedDependency = MHD_HTTP_FAILED_DEPENDENCY,
    UnorderedCollection = MHD_HTTP_UNORDERED_COLLECTION,
    UpgradeRequired = MHD_HTTP_UPGRADE_REQUIRED,
#if MHD_VERSION >= 0x00095800
    TooManyRequests = MHD_HTTP_TOO_MANY_REQUESTS,
    RequestHeaderFieldsTooLarge = MHD_HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE,
#endif // #if MHD_VERSION >= 0x00095800
    NoResponse = MHD_HTTP_NO_RESPONSE,
    RetryWith = MHD_HTTP_RETRY_WITH,
    BlockedByWindowsParentalControls = MHD_HTTP_BLOCKED_BY_WINDOWS_PARENTAL_CONTROLS,
    UnavailableForLegalReasons = MHD_HTTP_UNAVAILABLE_FOR_LEGAL_REASONS,

    InternalServerError = MHD_HTTP_INTERNAL_SERVER_ERROR,
    NotImplemented = MHD_HTTP_NOT_IMPLEMENTED,
    BadGateway = MHD_HTTP_BAD_GATEWAY,
    ServiceUnavailable = MHD_HTTP_SERVICE_UNAVAILABLE,
    GatewayTimeOut = MHD_HTTP_GATEWAY_TIMEOUT,
    HTTPVersionNotSupported = MHD_HTTP_HTTP_VERSION_NOT_SUPPORTED,
    VariantAlsoNegotiates = MHD_HTTP_VARIANT_ALSO_NEGOTIATES,
    InsufficientStorage = MHD_HTTP_INSUFFICIENT_STORAGE,
#if MHD_VERSION >= 0x00095800
    LoopDetected = MHD_HTTP_LOOP_DETECTED,
#endif // #if MHD_VERSION >= 0x00095800
    BandWidthExceeded = MHD_HTTP_BANDWIDTH_LIMIT_EXCEEDED,
    NotExtended = MHD_HTTP_NOT_EXTENDED,
#if MHD_VERSION >= 0x00095800
    NetworkAuthenticationRequired = MHD_HTTP_NETWORK_AUTHENTICATION_REQUIRED
#endif // #if MHD_VERSION >= 0x00095800
  };



  // Types
  // -----

  typedef enum MHD_ValueKind ValueKind;



  // File
  // ----

  class File
  {
    explicit File ( int fd ) : fd_( fd ) {}

  public:
    File () = default;

    File ( const char *fileName, int flags )
      : fd_( ::open( fileName, flags ) )
    {}

    File ( const std::string &fileName, int flags )
      : File( fileName.c_str(), flags )
    {}

    File ( const File & ) = delete;

    File ( File &&other )
      : fd_( other.fd_ )
    {
      other.fd_ = -1;
    }

    ~File () { close(); }

    File &operator= ( const File & ) = delete;
    File &operator= ( File &&other )
    {
      if( this == &other )
        return *this;
      close();
      fd_ = other.fd_;
      other.fd_ = -1;
    }

    operator bool () const { return (fd_ != -1); }

    bool open ( const char *fileName, int flags )
    {
      close();
      fd_ = ::open( fileName, flags );
      return (fd_ != -1);
    }

    void close ()
    {
      if( fd_ != -1 )
        ::close( fd_ );
      fd_ = -1;
    }

    off_t size () const
    {
      if( fd_ == -1 )
        throw std::invalid_argument( "File has not been opened." );
      struct stat info;
      if( fstat( fd_, &info ) == -1 )
        throw std::runtime_error( "Cannot obtain file size." );
      return info.st_size;
    }

    int releaseFD ()
    {
      int fd = fd_;
      fd_ = -1;
      return fd;
    }

    friend File makeFileFromFD ( int fd ) { return File( fd ); }

  private:
    int fd_ = -1;
  };



  // Buffer
  // ------

  class Buffer
  {
  public:
    Buffer ( const char *data, size_t size )
      : data_( data ), size_( size )
    {}

    const char *data () const { return data_; }

    size_t size () const { return size_; }

  private:
    const char *data_;
    size_t size_;
  };



  // RequestHandler
  // --------------

  class RequestHandler
  {
  public:
    virtual ~RequestHandler () = default;

    virtual bool operator() ( Connection connection, const char *uploadData, size_t *uploadDataSize ) = 0;
  };



  // Callback Functions
  // ------------------

  typedef std::function< int ( const struct sockaddr *addr, socklen_t addrlen ) > AcceptPolicy;

  typedef std::function< ssize_t ( uint64_t pos, char *buf, size_t max ) > ContentReader;

  typedef std::function< bool ( ValueKind kind, const char *key, const char *fileName, const char *contentType, const char *encoding, const char *data, uint64_t offset, size_t size ) > PostDataIterator;



  // Response
  // --------

  class Response
  {
    friend class Connection;

    struct Deleter
    {
      void operator() ( struct MHD_Response *handle ) { MHD_destroy_response( handle ); }
    };

  public:
    Response ( std::uint64_t size, std::size_t block_size, ContentReader contentReader )
      : handle_( MHD_create_response_from_callback( size, block_size, contentReaderCallback, new ContentReader( std::move( contentReader ) ), contentReaderFreeCallback ) )
    {
      if( !handle_ )
        throw std::runtime_error( "Cannot allocate response." );
    }

    Response ( std::size_t block_size, ContentReader contentReader )
      : Response( -1, block_size, contentReader )
    {}

    Response ( const void *data, std::size_t size, enum MHD_ResponseMemoryMode mode )
      : handle_( MHD_create_response_from_buffer( size, const_cast< void * >( data ), mode ) )
    {
      if( !handle_ )
        throw std::runtime_error( "Cannot allocate response." );
    }

    Response ( const char *data, enum MHD_ResponseMemoryMode mode = MHD_RESPMEM_MUST_COPY )
      : Response( data, std::strlen( data ), mode )
    {}

    Response ()
      : Response( "", 0, MHD_RESPMEM_PERSISTENT )
    {}

    template< class T, std::size_t size = sizeof( T ) >
    explicit Response ( std::unique_ptr< T > data )
      : Response( data.release(), size, MHD_RESPMEM_MUST_FREE )
    {}

    explicit Response ( const std::string &s )
      : Response( s.data(), s.size(), MHD_RESPMEM_MUST_COPY )
    {}

    Response ( File file, size_t size )
      : handle_( MHD_create_response_from_fd( size, file.releaseFD() ) )
    {
      if( !handle_ )
        throw std::runtime_error( "Cannot allocate response." );
    }

    explicit Response ( File file )
    {
      const size_t size = file.size();
      handle_.reset( MHD_create_response_from_fd( size, file.releaseFD() ) );
      if( !handle_ )
        throw std::runtime_error( "Cannot allocate response." );
    }

    bool addHeader ( const char *header, const char *content ) { return (MHD_add_response_header( handle_.get(), header, content ) != MHD_NO); }
    bool addHeader ( const char *header, const std::string &content ) { return addHeader( header, content.c_str() ); }
    bool addHeader ( const std::string &header, const char *content ) { return addHeader( header.c_str(), content ); }
    bool addHeader ( const std::string &header, const std::string &content ) { return addHeader( header.c_str(), content.c_str() ); }

    template< class T >
    bool addHeader ( std::pair< T, T > value ) { return addHeader( value.first, value.second ); }

    template< class Iterator >
    auto addHeaders ( Iterator begin, Iterator end )
      -> std::enable_if_t< std::is_convertible< decltype( *begin ), std::pair< const char *, const char * > >::value || std::is_convertible< decltype( *begin ), std::pair< std::string, std::string > >::value, Iterator >
    {
      for( ; (begin != end) && addHeader( *begin ); ++begin )
        continue;
      return begin;
    }

    template< class Range >
    auto addHeaders ( const Range &range )
      -> std::enable_if_t< std::is_convertible< decltype( *range.begin() ), std::pair< const char *, const char * > >::value >
    {
      return addHeaders( range.begin(), range.end() );
    }

    static Response makeRedirect ( const std::string &url )
    {
      Response response;
      response.addHeader( MHD_HTTP_HEADER_LOCATION, url );
      return response;
    }


    template< class... Args, std::enable_if_t< std::is_constructible< Response, Args &&... >::value, int > = 0 >
    static Response makeContentResponse ( const std::string &contentType, Args &&... args )
    {
      Response response( std::forward< Args >( args )... );
      response.addHeader( MHD_HTTP_HEADER_CONTENT_TYPE, contentType.c_str() );
      return response;
    }

  private:
    static ssize_t contentReaderCallback ( void *cls, uint64_t pos, char *buf, size_t max )
    {
      ContentReader &contentReader = *static_cast< ContentReader * >( cls );
      return contentReader( pos, buf, max );
    }

    static void contentReaderFreeCallback ( void *cls ) { delete static_cast< ContentReader * >( cls ); }

    std::unique_ptr< struct MHD_Response, Deleter > handle_;
  };



  // Connection
  // ----------

  class Connection
  {
    friend class Daemon;
    friend class PostProcessor;

    Connection ( MHD_Connection *handle ) : handle_( handle ) {}

  public:
    template< class F >
    void forEachValue ( ValueKind kind, F &&f )
    {
      MHD_get_connection_values( handle_, kind, keyValueIterator< F >, &f );
    }

    const char *lookupValue ( ValueKind kind, std::string key ) const
    {
      return MHD_lookup_connection_value( handle_, kind, key.c_str() );
    }

    bool queue ( StatusCode statusCode, const Response &response )
    {
      return (MHD_queue_response( handle_, static_cast< unsigned int >( statusCode ), response.handle_.get() ) != MHD_NO);
    }

  private:
    template< class F >
    static auto keyValueIterator ( void *cls, ValueKind kind, const char *key, const char *value )
      -> std::enable_if_t< std::is_same< decltype( std::declval< F & >()( kind, std::make_pair( key, value ) ) ), void >::value, int >
    {
      F &f = *static_cast< F * >( cls );
      f( kind, std::make_pair( key, value ) );
      return MHD_YES;
    }

    template< class F >
    static auto keyValueIterator ( void *cls, ValueKind kind, const char *key, const char *value )
      -> std::enable_if_t< std::is_same< decltype( std::declval< F & >()( kind, std::make_pair( key, value ) ) ), bool >::value, int >
    {
      F &f = *static_cast< F * >( cls );
      return (f( kind, std::make_pair( key, value ) ) ? MHD_YES : MHD_NO);
    }

    template< class F >
    static auto keyValueIterator ( void *cls, ValueKind kind, const char *key, const char *value )
      -> std::enable_if_t< std::is_same< decltype( std::declval< F & >()( std::make_pair( key, value ) ) ), void >::value, int >
    {
      F &f = *static_cast< F * >( cls );
      f( std::make_pair( key, value ) );
      return MHD_YES;
    }

    template< class F >
    static auto keyValueIterator ( void *cls, ValueKind kind, const char *key, const char *value )
      -> std::enable_if_t< std::is_same< decltype( std::declval< F & >()( std::make_pair( key, value ) ) ), bool >::value, int >
    {
      F &f = *static_cast< F * >( cls );
      return (f( std::make_pair( key, value ) ) ? MHD_YES : MHD_NO);
    }

    struct MHD_Connection *handle_;
  };



  // PostProcessor
  // -------------

  class PostProcessor
  {
  public:
    PostProcessor ( Connection connection, size_t bufferSize, PostDataIterator iterator )
      : iterator_( std::move( iterator ) ),
        handle_( MHD_create_post_processor( connection.handle_, bufferSize, iteratorCallback, this ) )
    {}

    PostProcessor ( Connection connection, PostDataIterator iterator )
      : PostProcessor( connection, 65536, iterator )
    {}

    PostProcessor ( const PostProcessor & ) = delete;
    PostProcessor ( PostProcessor && ) = delete;

    ~PostProcessor () { MHD_destroy_post_processor( handle_ ); }

    PostProcessor &operator= ( const PostProcessor & ) = delete;
    PostProcessor &operator= ( PostProcessor && ) = delete;

    bool operator() ( Buffer buffer ) { return static_cast< bool >( MHD_post_process( handle_, buffer.data(), buffer.size() ) ); }

  private:
    static int iteratorCallback ( void *cls, ValueKind kind, const char *key, const char *fileName, const char *contentType, const char *encoding, const char *data, uint64_t offset, size_t size )
    {
      PostProcessor &postProcessor = *static_cast< PostProcessor * >( cls );
      return postProcessor.iterator_( kind, key, fileName, contentType, encoding, data, offset, size );
    }

    PostDataIterator iterator_;
    struct MHD_PostProcessor *handle_;
  };



  // ResponseRequestHandler
  // ----------------------

  class ResponseRequestHandler
    : public RequestHandler
  {
  public:
    template< class... Args, std::enable_if_t< std::is_constructible< Response, Args &&... >::value, int > = 0 >
    explicit ResponseRequestHandler ( StatusCode statusCode, Args &&... args )
      : statusCode_( statusCode ), response_( std::forward< Args >( args )... )
    {}

    bool operator() ( Connection connection, const char *uploadData, size_t *uploadDataSize ) override
    {
      return connection.queue( statusCode_, response_ );
    }

  private:
    StatusCode statusCode_;
    Response response_;
  };



  // makeResponseRequestHandler
  // --------------------------

  template< class... Args >
  inline static auto makeResponseRequestHandler ( Args &&... args )
    -> std::enable_if_t< std::is_constructible< ResponseRequestHandler, Args &&... >::value, std::unique_ptr< RequestHandler > >
  {
    return std::make_unique< ResponseRequestHandler >( std::forward< Args >( args )... );
  }

  template< class... Args >
  inline static auto makeNotFoundRequestHandler ( Args &&... args )
    -> std::enable_if_t< std::is_constructible< ResponseRequestHandler, StatusCode, Args &&... >::value, std::unique_ptr< RequestHandler > >
  {
    return std::make_unique< ResponseRequestHandler >( StatusCode::NotFound, std::forward< Args >( args )... );
  }

  template< class... Args >
  inline static auto makeMethodNotAllowedRequestHandler ( Args &&... args )
    -> std::enable_if_t< std::is_constructible< ResponseRequestHandler, StatusCode, Args &&... >::value, std::unique_ptr< RequestHandler > >
  {
    return std::make_unique< ResponseRequestHandler >( StatusCode::MethodNotAllowed, std::forward< Args >( args )... );
  }

  inline static auto makeRedirectRequestHandler ( StatusCode statusCode, const std::string &url )
  {
    return makeResponseRequestHandler( statusCode, Response::makeRedirect( url ) );
  }

  inline static auto makeRedirectRequestHandler ( const std::string &url )
  {
    return makeRedirectRequestHandler( StatusCode::Found, url );
  }

  template< class... Args >
  inline static auto makeContentRequestHandler ( StatusCode statusCode, const std::string &contentType, Args &&... args )
    -> std::enable_if_t< std::is_constructible< Response, Args &&... >::value, std::unique_ptr< RequestHandler > >
  {
    return makeResponseRequestHandler( statusCode, Response::makeContentResponse( contentType, std::forward< Args >( args )... ) );
  }

  template< class... Args >
  inline static auto makeContentRequestHandler ( const std::string &contentType, Args &&... args )
    -> std::enable_if_t< std::is_constructible< Response, Args &&... >::value, std::unique_ptr< RequestHandler > >
  {
    return makeContentRequestHandler( StatusCode::Ok, contentType, std::forward< Args >( args )... );
  }



  // PostRequestHandler
  // ------------------

  class PostRequestHandler
    : public RequestHandler
  {
  protected:
    struct Field
    {
      std::string fileName;
      std::string contentType;
      std::string encoding;
      std::string data;
    };

    explicit PostRequestHandler ( httpd::Connection connection )
      : postProcessor_( connection, [ this ] ( httpd::ValueKind kind, const char *key, const char *fileName, const char *contentType, const char *encoding, const char *data, uint64_t offset, size_t size ) {
            const auto result = fields_.emplace( key, Field() );
            if( result.second )
            {
              result.first->second.fileName = (fileName ? fileName : "");
              result.first->second.contentType = (contentType ? contentType : "");
              result.first->second.encoding = (encoding ? encoding : "");
            }
            result.first->second.data.append( data, size );
            return true;
          } )
    {}

  public:
    bool operator() ( httpd::Connection connection, const char *uploadData, size_t *uploadDataSize ) final override
    {
      if( *uploadDataSize == 0 )
        return (*this)( connection );
      postProcessor_( httpd::Buffer( uploadData, *uploadDataSize ) );
      *uploadDataSize = 0;
      return true;
    }

    virtual bool operator() ( httpd::Connection connection ) = 0;

  protected:
    const Field *field ( const std::string &key )
    {
      const auto pos = fields_.find( key );
      return (pos != fields_.end() ? &pos->second : nullptr);
    }

  private:
    httpd::PostProcessor postProcessor_;
    std::map< std::string, Field > fields_;
  };



  // Daemon
  // ------

  class Daemon
  {
  protected:
    Daemon ( unsigned int flags, unsigned short port )
      : handle_( MHD_start_daemon( flags, port, nullptr, nullptr, accessHandlerCallback, this,
                                   MHD_OPTION_NOTIFY_COMPLETED, requestCompletedCallback, this,
                                   MHD_OPTION_END ) )
    {
      if( !handle_ )
        throw std::runtime_error( "Cannot create daemon." );
    }

    Daemon ( unsigned int flags, unsigned short port, AcceptPolicy acceptPolicy )
      : acceptPolicy_( std::move( acceptPolicy ) ),
        handle_( MHD_start_daemon( flags, port, acceptPolicyCallback, this, accessHandlerCallback, this,
                                   MHD_OPTION_NOTIFY_COMPLETED, requestCompletedCallback, this,
                                   MHD_OPTION_END ) )
    {
      if( !handle_ )
        throw std::runtime_error( "Cannot create daemon." );
    }

  public:
    Daemon ( const Daemon & ) = delete;
    Daemon ( Daemon && ) = delete;

    virtual ~Daemon () { MHD_stop_daemon( handle_ ); }

    Daemon &operator= ( const Daemon & ) = delete;
    Daemon &operator= ( Daemon && ) = delete;

    virtual std::unique_ptr< RequestHandler > getRequestHandler ( Connection connection, const char *url, const char *method, const char *version ) = 0;

  private:
    static int acceptPolicyCallback ( void *cls, const struct sockaddr *addr, socklen_t addrlen )
    {
      Daemon &daemon = *static_cast< Daemon * >( cls );
      return daemon.acceptPolicy_( addr, addrlen );
    }

    static int accessHandlerCallback ( void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *uploadData, size_t *uploadDataSize, void **con_cls )
    {
      if( *con_cls )
      {
        // this is an established connection; just call the request handler
        RequestHandler &requestHandler = *static_cast< RequestHandler * >( *con_cls );
        return (requestHandler( connection, uploadData, uploadDataSize ) ? MHD_YES : MHD_NO);
      }
      else
      {
        // this is a new connection; create a request handler
        Daemon &daemon = *static_cast< Daemon * >( cls );
        std::unique_ptr< RequestHandler > requestHandler = daemon.getRequestHandler( connection, url, method, version );
        *con_cls = requestHandler.release();
        assert( *uploadDataSize == 0 );
        return (*con_cls ? MHD_YES : MHD_NO);
      }
    }

    static void requestCompletedCallback ( void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe )
    {
      RequestHandler *requestHandler = static_cast< RequestHandler * >( *con_cls );
      delete requestHandler;
      *con_cls = nullptr;
    }

    AcceptPolicy acceptPolicy_;
    struct MHD_Daemon *handle_ = nullptr;
  };

} // namespace httpd

#endif // #ifndef HTTPD_HH
