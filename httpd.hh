#ifndef HTTPD_HH
#define HTTPD_HH

#include <cstddef>
#include <cstdint>
#include <cstring>

#include <functional>
#include <memory>
#include <string>
#include <utility>

#include <microhttpd.h>

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



  // Callback Functions
  // ------------------

  typedef std::function< int ( const struct sockaddr *addr, socklen_t addrlen ) > AcceptPolicy;

  typedef std::function< int ( Connection connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls ) > AccessHandler;

  typedef std::function< ssize_t ( uint64_t pos, char *buf, size_t max ) > ContentReader;



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
        std::terminate();
    }

    Response ( const void *data, std::size_t size, enum MHD_ResponseMemoryMode mode )
      : handle_( MHD_create_response_from_buffer( size, const_cast< void * >( data ), mode ) )
    {
      if( !handle_ )
        std::terminate();
    }

    Response ( const char *data, enum MHD_ResponseMemoryMode mode = MHD_RESPMEM_MUST_COPY )
      : Response( data, std::strlen( data ), mode )
    {}

    template< class T, std::size_t size = sizeof( T ) >
    explicit Response ( std::unique_ptr< T > data )
      : Response( data.release(), size, MHD_RESPMEM_MUST_FREE )
    {}

    explicit Response ( const std::string &s )
      : Response( s.data(), s.size(), MHD_RESPMEM_MUST_COPY )
    {}

    bool addHeader ( const char *header, const char *content ) { return (MHD_add_response_header( handle_.get(), header, content ) != MHD_NO); }
    bool addHeader ( std::pair< const char *, const char * > value ) { return addHeader( value.first, value.second ); }

    template< class Iterator >
    auto addHeaders ( Iterator begin, Iterator end )
      -> std::enable_if_t< std::is_convertible< decltype( *begin ), std::pair< const char *, const char * > >::value, Iterator >
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

    Connection ( MHD_Connection *handle ) : handle_( handle ) {}

  public:
    template< class F >
    void forEachValue ( ValueKind kind, F &&f )
    {
      MHD_get_connection_values( handle_, kind, keyValueIterator< F >, &f );
    }

    const char *lookupValue ( ValueKind kind, std::string key )
    {
      return MHD_lookup_connection_value( handle_, kind, key.c_str() );
    }

    int queue ( StatusCode statusCode, const Response &response )
    {
      return MHD_queue_response( handle_, static_cast< unsigned int >( statusCode ), response.handle_.get() );
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



  // Daemon
  // ------

  class Daemon
  {
  public:
    Daemon ( unsigned int flags, unsigned short port, AccessHandler accessHandler )
      : accessHandler_( std::move( accessHandler ) ),
        handle_( MHD_start_daemon( flags, port, nullptr, nullptr, accessHandlerCallback, this, MHD_OPTION_END ) )
    {
      if( !handle_ )
        std::terminate();
    }

    Daemon ( unsigned int flags, unsigned short port, AcceptPolicy acceptPolicy, AccessHandler accessHandler )
      : acceptPolicy_( std::move( acceptPolicy ) ),
        accessHandler_( std::move( accessHandler ) ),
        handle_( MHD_start_daemon( flags, port, acceptPolicyCallback, this, accessHandlerCallback, this, MHD_OPTION_END ) )
    {
      if( !handle_ )
        std::terminate();
    }

    Daemon ( const Daemon & ) = delete;
    Daemon ( Daemon && ) = delete;

    ~Daemon () { MHD_stop_daemon( handle_ ); }

    Daemon &operator= ( const Daemon & ) = delete;
    Daemon &operator= ( Daemon && ) = delete;

  private:
    static int acceptPolicyCallback ( void *cls, const struct sockaddr *addr, socklen_t addrlen )
    {
      Daemon &daemon = *static_cast< Daemon * >( cls );
      return daemon.acceptPolicy_( addr, addrlen );
    }

    static int accessHandlerCallback ( void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls )
    {
      Daemon &daemon = *static_cast< Daemon * >( cls );
      return daemon.accessHandler_( connection, url, method, version, upload_data, upload_data_size, con_cls );
    }

    AcceptPolicy acceptPolicy_;
    AccessHandler accessHandler_;
    struct MHD_Daemon *handle_ = nullptr;
  };



  // PostProcessor
  // -------------

  class PostProcessor
  {
  };

} // namespace httpd

#endif // #ifndef HTTPD_HH
