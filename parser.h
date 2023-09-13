#define PARSER_VERSION_MAJOR 
#define PARSER_VERSION_MINOR 
#define PARSER_VERSION_PATCH 
#define PARSER_H
#include <cstddef>
#include <map>

enum class Httpparserstate {
    IDLE,
    REQUEST_LINE,
    HEADERS,
    BODY,
    FINISHED,
    PARSER_ERROR,
    s_dead = 1 
  , s_start_req_or_res
  , s_res_or_resp_H
  , s_start_res
  , s_res_H
  , s_res_HT
  , s_res_HTT
  , s_res_HTTP
  , s_res_http_major
  , s_res_http_dot
  , s_res_http_minor
  , s_res_http_end
  , s_res_first_status_code
  , s_res_status_code
  , s_res_status_start
  , s_res_status
  , s_res_line_almost_done

  , s_start_req

  , s_req_method
  , s_req_spaces_before_url
  , s_req_schema
  , s_req_schema_slash
  , s_req_schema_slash_slash
  , s_req_server_start
  , s_req_server
  , s_req_server_with_at
  , s_req_path
  , s_req_query_string_start
  , s_req_query_string
  , s_req_fragment_start
  , s_req_fragment
  , s_req_http_start
  , s_req_http_H
  , s_req_http_HT
  , s_req_http_HTT
  , s_req_http_HTTP
  , s_req_http_I
  , s_req_http_IC
  , s_req_http_major
  , s_req_http_dot
  , s_req_http_minor
  , s_req_http_end
  , s_req_line_almost_done

  , s_header_field_start
  , s_header_field
  , s_header_value_discard_ws
  , s_header_value_discard_ws_almost_done
  , s_header_value_discard_lws
  , s_header_value_start
  , s_header_value
  , s_header_value_lws

  , s_header_almost_done

  , s_chunk_size_start
  , s_chunk_size
  , s_chunk_parameters
  , s_chunk_size_almost_done

  , s_headers_almost_done
  , s_headers_done

  , s_chunk_data
  , s_chunk_data_almost_done
  , s_chunk_data_done

  , s_body_identity
  , s_body_identity_eof

  , s_message_done
};

enum class HttpMessageType {
    REQUEST,
    RESPONSE
};

#define HTTP_ERRNO_MAP(XX)                                           \
  /* No error */                                                     \
  XX(OK, "success")                                                  \
                                                                     \
  /* Callback-related errors */                                      \
  XX(CB_message_begin, "the on_message_begin callback failed")       \
  XX(CB_url, "the on_url callback failed")                           \
  XX(CB_header_field, "the on_header_field callback failed")         \
  XX(CB_header_value, "the on_header_value callback failed")         \
  XX(CB_headers_complete, "the on_headers_complete callback failed") \
  XX(CB_body, "the on_body callback failed")                         \
  XX(CB_message_complete, "the on_message_complete callback failed") \
  XX(CB_status, "the on_status callback failed")                     \
  XX(CB_chunk_header, "the on_chunk_header callback failed")         \
  XX(CB_chunk_complete, "the on_chunk_complete callback failed")     \
                                                                     \
  /* Parsing-related errors */                                       \
  XX(INVALID_EOF_STATE, "stream ended at an unexpected time")        \
  XX(HEADER_OVERFLOW,                                                \
     "too many header bytes seen; overflow detected")                \
  XX(CLOSED_CONNECTION,                                              \
     "data received after completed connection: close message")      \
  XX(INVALID_VERSION, "invalid HTTP version")                        \
  XX(INVALID_STATUS, "invalid HTTP status code")                     \
  XX(INVALID_METHOD, "invalid HTTP method")                          \
  XX(INVALID_URL, "invalid URL")                                     \
  XX(INVALID_HOST, "invalid host")                                   \
  XX(INVALID_PORT, "invalid port")                                   \
  XX(INVALID_PATH, "invalid path")                                   \
  XX(INVALID_QUERY_STRING, "invalid query string")                   \
  XX(INVALID_FRAGMENT, "invalid fragment")                           \
  XX(LF_EXPECTED, "LF character expected")                           \
  XX(INVALID_HEADER_TOKEN, "invalid character in header")            \
  XX(INVALID_CONTENT_LENGTH,                                         \
     "invalid character in content-length header")                   \
  XX(UNEXPECTED_CONTENT_LENGTH,                                      \
     "unexpected content-length header")                             \
  XX(INVALID_CHUNK_SIZE,                                             \
     "invalid character in chunk size header")                       \
  XX(INVALID_CONSTANT, "invalid constant string")                    \
  XX(INVALID_INTERNAL_STATE, "encountered unexpected internal state")\
  XX(STRICT, "strict mode assertion failed")                         \
  XX(PAUSED, "parser is paused")                                     \
  XX(UNKNOWN, "an unknown error occurred")                           \
  XX(INVALID_TRANSFER_ENCODING,                                      \
     "request has invalid transfer-encoding")                        \


/* Define HPE_* values for each errno value above */
#define HTTP_ERRNO_GEN(n, s) HPE_##n,
enum http_parser_error {
  HTTP_ERRNO_MAP(HTTP_ERRNO_GEN)
};
#undef HTTP_ERRNO_GEN

#define HTTP_PARSER_ERRNO(p) ((enum http_parser_error) (p)->http_parser_error)


//start line/header/body

struct Httpparsersettings {
  void (*http_on_Message_Begin)(); 
  // 메시지 시작
  void (*http_on_Url)(const char *url, size_t length);
  //URL 파싱할때
  void (*http_on_status)(const char *status, size_t length);
  //상태코드 파싱할떄
  void (*http_on_Header_Field)(const char *field, size_t length);
  //헤더필드 파싱
  void (*http_on_Header_Value)(const char *value, size_t length);
  //헤더값 파싱
  void (*http_on_Headers_Complete)();
  //헤더완료
  void (*http_on_Body)(const char *body, size_t length);
  //진짜 메시지 파싱
  void (*http_on_Message_Complete)();
  //메시지 완료
};


enum class Http_status{                                                \
  CONTINUE=100,
  SWITCHING_PROTOCOLS = 101,
  PROCESSING = 102,
  OK = 200,
  CREATED=201,
  ACCEPTED=202,
  NON_AUTHORITATIVE_INFORMATION=203,
  NO_CONTENT=204,
  RESET_CONTENT=205,
  PARTIAL_CONTENT=206,
  MULTI_STATUS=207,
  ALREADY_REPORTED=208,
  IM_USED=226,
  MULTIPLE_CHOICES=300,
  MOVED_PERMANENTLY=301,
  FOUND=302,
  SEE_OTHER=303,
  NOT_MODIFIED=304,
  USE_PROXY=305,
  TEMPORARY_REDIRECT=307,
  PERMANENT_REDIRECT=308,
  BAD_REQUEST=400,
  UNAUTHORIZED=401,
  PAYMENT_REQUIRED=402,
  FORBIDDEN=403,
  NOT_FOUND=404,
  METHOD_NOT_ALLOWED=405,
  NOT_ACCEPTABLE=406,
  PROXY_AUTHENTICATION_REQUIRED=407,
  REQUEST_TIMEOUT=408,
  CONFLICT=409,
  GONE=410,
  LENGTH_REQUIRED=411,
  PRECONDITION_FAILED=412,
  PAYLOAD_TOO_LARGE=413,
  URI_TOO_LONG=414,
  UNSUPPORTED_MEDIA_TYPE=415,
  RANGE_NOT_SATISFIABLE=416,
  EXPECTATION_FAILED=417,
  MISDIRECTED_REQUES=421,
  UNPROCESSABLE_ENTITY=422,
  LOCKED=423,
  FAILED_DEPENDENCY=424,
  UPGRADE_REQUIRED=426,
  PRECONDITION_REQUIRED=428,
  TOO_MANY_REQUESTS=429,
  REQUEST_HEADER_FIELDS_TOO_LARGE=431,
  UNAVAILABLE_FOR_LEGAL_REASONS=451,
  INTERNAL_SERVER_ERROR=500,
  NOT_IMPLEMENTED=501,
  BAD_GATEWAY=502,
  SERVICE_UNAVAILABLE=503,
  GATEWAY_TIMEOUT=504,
  HTTP_VERSION_NOT_SUPPORTED=505,
  VARIANT_ALSO_NEGOTIATES=506,
  INSUFFICIENT_STORAGE=507,
  LOOP_DETECTED=508, 
  NOT_EXTENDED=510,
  NETWORK_AUTHENTICATION_REQUIRED=511
};

enum Http_method{
  DELETE,GET,HEAD,POST,PUT,CONNECT,OPTIONS,TRACE,
  COPY,LOCK,MKCOL,MOVE,PROPFIND,PROPPATCH,SEARCH,
  UNLOCK, BIND, REBIND, UNBIND, ACL, REPORT, MKACTIVITY,
  CHECKOUT, MERGE, MSEARCH, NOTIFY, SUBSCRIBE, UNSUBSCRIBE, 
  PATCH,PURGE, MKCALENDAR,LINK,UNLINK,SOURCE, UNDEFINED
};



struct Httpparser {
    Httpparserstate state;
    Http_method method;
    std::string startLine;
    std::vector<std::string> headers;
    std::string bodydata;
    size_t nread;
};

void http_parser_init(Httpparser *parser, Http_method method);
int http_parser_execute(Httpparser *parser, const Httpparsersettings *settings, const char *data, size_t len);
int http_should_keep_alive(const Httpparser *parser);
void httpdivide(Httpparser& parser, const char *data, std::string& startLine, std::vector<std::string>& headers, std::string& bodydata);
void parseHeaders(const std::vector<std::string>& headers, std::string& transferencoding, size_t& contentLength);
void Errorhandle(Httpparser *parser, int error);
