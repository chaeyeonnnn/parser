#include <iostream>
#include <string>
#include "parser.h"
#define KEEP_ALIVE "keep-alive"
#define CLOSE "close"
#define CONTENT_LENGTH "content-length"
#define PROXY_CONNECTION "proxy-connection"
#define CONNECTION "connection"
#include <sstream>

/*
#ifndef BIT_AT
# define BIT_AT(a, i)                                                \
  (!!((unsigned int) (a)[(unsigned int) (i) >> 3] &                  \
   (1 << ((unsigned int) (i) & 7))))
#endif
*/


#define LOWER(c)            (unsigned char)(c | 0x20)
#define IS_ALPHA(c)         (LOWER(c) >= 'a' && LOWER(c) <= 'z')
#define IS_NUM(c)           ((c) >= '0' && (c) <= '9')
#define IS_ALPHANUM(c)      (IS_ALPHA(c) || IS_NUM(c))
#define IS_MARK(c)          ((c) == '-' || (c) == '_' || (c) == '.' || \
  (c) == '!' || (c) == '~' || (c) == '*' || (c) == '\'' || (c) == '(' || \
  (c) == ')')
#define IS_USERINFO_CHAR(c) (IS_ALPHANUM(c) || IS_MARK(c) || (c) == '%' || \
  (c) == ';' || (c) == ':' || (c) == '&' || (c) == '=' || (c) == '+' || \
  (c) == '$' || (c) == ',')

#define CURRENT_STATE() p_state
#define UPDATE_STATE(V) p_state = (enum state) (V);
#define RETURN(V)                                                    \
do {                                                                 \
  parser->nread = nread;                                             \
  parser->state = CURRENT_STATE();                                   \
  return (V);                                                        \
} while (0);
#define REEXECUTE()                                                  \
  goto reexecute;  

#define CR '\r'
#define LF '\n'
#define MAX_BODY_SIZE

#define IS_URL_CHAR(c)      (BIT_AT(normal_url_char, (unsigned char)c))
#define IS_URL_CHAR(c)                                                         \
  (BIT_AT(normal_url_char, (unsigned char)c) || ((c) & 0x80))


#define SET_ERRNO(e)                                                 \
do {                                                                 \
  parser->nread = nread;                                             \
  parser->http_errno = (e);                                         \
} while(0)

#define CALLBACK_NOTIFY_NOADVANCE(FOR)  CALLBACK_NOTIFY_(FOR, p - data)
#define COUNT_HEADER_SIZE(V)                                         \
do {                                                                 \
  nread += (uint32_t)(V);                                            \
  if (UNLIKELY(nread > max_header_size)) {                           \
    SET_ERRNO(HPE_HEADER_OVERFLOW);                                  \
    goto error;                                                      \
  }                                                                  \
} while (0)



void http_on_Message_Begin(){
    std::cout << "Message Begin" << std::endl;
};

void http_on_Url(const char *url, size_t length){
    std::cout << "URL: " << std::string(url, length) << std::endl;
};

void http_on_status(const char *status, size_t length){
    std::cout << "status code: " << std::string(status, length) << std::endl;
};

void http_on_Header_Field(const char *field, size_t length) {
    std::vector<std::string> header;
    header.emplace_back(field, length);  // field를 벡터에 추가
    std::cout << "Header Field: " << std::string(field, length) << std::endl;
};

void http_on_Header_Value( const char *value, size_t length){
    std::cout << "Header Value: " << std::string(value, length) << std::endl;
};

void http_on_Headers_Complete(){
    std::cout << "Headers Complete" << std::endl;
};

void http_on_Body( const char *body, size_t length){
    std::cout << "Body: " << std::string(body, length) << std::endl;
};

void http_onMessageComplete(){
    std::cout << "Message Complete" << std::endl;
};

/* 구조체 다 클래스로 바꿀 것
class HttpRequest {
  std::string method;
  std::string url;
  std::string HTTP_version;
  std::map<std::string, std::string> headers;
  public:
      std::string body;
};

class HttpResponse {
  std::string HTTP_version;
  std::string status_code;
  std::string status_message;
  std::map<std::string, std::string> headers;
  public:
      std::string body;

};
*/


// HTTP 파싱 오류 처리 함수
void Errorhandle(Httpparser *parser, int error) {
    switch (error) {
        case HPE_OK:
            break;
        case HPE_INVALID_CONTENT_LENGTH:
            // 헤더랑 본문 길이 안맞는 경우 헤더 파싱할떄
            fprintf(stderr, "Error: Invalid Content-Length\n");
            break;
        case HPE_CB_message_begin:
            // 메시지시작 콜백 오류
            fprintf(stderr, "Error: Message Begin Callback Error\n");
            break;
        case HPE_CB_headers_complete:
            // 헤더완료 콜백 오류
            fprintf(stderr, "Error: Headers Complete Callback Error\n");
            break;
        case HPE_CB_message_complete:
            // 메시지완료 콜백 오류
            fprintf(stderr, "Error: Message Complete Callback Error\n");
            break;
        case HPE_INVALID_HEADER_TOKEN:
            // 잘못된 헤더 토큰
            fprintf(stderr, "Error: Invalid Header Token\n");
            break;
        case HPE_INVALID_VERSION:
            // 잘못된 HTTP 버전
            fprintf(stderr, "Error: Invalid HTTP Version\n");
            break;
        case HPE_INVALID_STATUS:
            // 잘못된 상태 코드
            fprintf(stderr, "Error: Invalid Status Code\n");
            break;
        // 다른 오류 유형에 대한 처리 추가
        default:
            fprintf(stderr, "Error: Unknown Error\n");
            break;
    }
}

//파서랑 메서드 초기화 때리고
void http_parser_init(Httpparser *parser, Http_method method){
    parser->state = Httpparserstate::IDLE;
    parser->method = method;
    parser->startLine.clear();
    parser->headers.clear();
    parser->bodydata.clear();
    parser->nread = 0; // nread 변수 초기화

};

// switch 문으로 바꿔?
Http_method parser_http_method(const std::string& method) {
    if (method == "GET") {
        return Http_method::GET;
    } else if (method == "POST") {
        return Http_method::POST;
    } else if (method == "PUT") {
        return Http_method::PUT;
    } else if (method == "DELETE") {
        return Http_method::DELETE;
    } else if (method == "HEAD"){
        return Http_method::HEAD;
    } else if (method == "OPTIONS"){
        return Http_method::OPTIONS; 
    } else if (method == "PATCH"){
        return Http_method::PATCH; 
    } else if (method == "CONNECT"){
        return Http_method::CONNECT;
    } else if (method == "TRACE"){
        return Http_method::TRACE;  
    } else {
        // 알 수 없는 메서드일 경우, 기본값으로 UNDEFINED 반환 또는 다른 처리를 수행
        return Http_method::UNDEFINED;
    }
}


// 메시지가 오면 startline, header, body 나누는 함수
void httpdivide(Httpparser& parser, const char *data, std::string& startLine, std::vector<std::string>& headers, std::string& bodydata) {
    // 빈 줄로 헤더와 본문을 분리
    size_t headerEnd = std::string(data).find("\r\n\r\n");
    // 다음이 두줄 띄어쓰기 있다? 그게 헤더의 끝

    if (headerEnd != std::string::npos) {
        // 시작 라인 추출
        size_t startLineEnd = std::string(data).find("\r\n");
        if (startLineEnd != std::string::npos) {
            startLine = std::string(data).substr(0, startLineEnd);
        }
        // 헤더 추출
        std::string headerSection = std::string(data).substr(startLineEnd + 2, headerEnd - startLineEnd - 2);
        size_t pos = 0; //헤더 필드 시작 위치
        while (pos != std::string::npos) {
            size_t lineEnd = headerSection.find("\r\n", pos);
            if (lineEnd != std::string::npos) {
                headers.push_back(headerSection.substr(pos, lineEnd - pos));
                pos = lineEnd + 2;
            } else {
                // 마지막 헤더
                headers.push_back(headerSection.substr(pos));
                pos = std::string::npos;
            }
        }
        // 본문 추출
        bodydata = std::string(data).substr(headerEnd + 4);
    }
}


int http_parser_execute(Httpparser *parser, const Httpparsersettings *settings, const char *data, size_t len){
    char ch;
    int8_t unhex_val; //URL 인코딩됨 문자 해독된 값 저장
    const char *p = data; //문자열 포인터 시작위치로 초기화    
    const char *header_field_mark = nullptr; //헤더 필드
    const char *header_value_mark = nullptr; //헤더 값
    const char *url_mark = nullptr; //URL 시작위치
    const char *body_mark = nullptr; //본문 시작위치
    const char *status_mark = nullptr; //상태코드+메시지 위치
    Httpparserstate p_state = static_cast<Httpparserstate>(parser->state);
    //파서의 현재 상태,, 지금 어디 처리중인가->형변환 오류 안나게

    http_on_Message_Begin();

    std::string startLine;
    std::vector<std::string> headers;
    std::string bodydata;

    httpdivide(*parser, data, startLine, headers, bodydata);
    //여기서 스타트라인-헤더-비디 가르고

    std::cout << "StartLine: " << startLine << std::endl;

    std::string message_type;
        
    if (startLine[0] == 'H'){
        if (startLine[1]=='T'){
            message_type = "request";}
        else
            message_type = "response";}
    else
        message_type = "response";
    //여기에서 startline 읽고 메시지 유형 결정

    std::cout << "Message Type: " << message_type << std::endl;

    if (message_type == "request") {
        std::istringstream stream(startLine);
        std::string Method, URI, HTTP_version;
        stream >> Method >> URI >> HTTP_version;

        parser->method = parser_http_method(Method);

        url_mark = data + std::string(data).find(URI);
        size_t length = URI.length();
        http_on_Url(URI.c_str(), length);
        std::cout << "HTTP_version: " << HTTP_version;

        header_field_mark = data + startLine.size() + 2;
        parser->state = Httpparserstate::HEADERS;
      } 

    else {
        std::istringstream stream(startLine);
        std::string HTTP_version, Http_status, http_message;
        stream >> HTTP_version >> Http_status;
        std::getline(stream,http_message);

        status_mark = data+startLine.find(Http_status);        
        http_on_status(Http_status.c_str(), Http_status.length());
        std::cout << "http message: " << http_message << std::endl; 

        header_field_mark = data + startLine.size() + 2; //헤더필드 시작하는 위치
        parser->state = Httpparserstate::HEADERS;
      }

    std::string transferencoding;
    size_t contentLength = 0;

    size_t startlineEnd = startLine.find('\r\n');
    //스타트라인 끝난 위치

    const char *message_end = strstr(data, "\r\n\r\n");
    //message_end : 헤더가 끝난 위치 먼저 선언하고
    
    if (message_end != nullptr) {
        body_mark = message_end + 4; // 헤더 끝난 자리 다음다음부터 바디 시작할
        parser->state = Httpparserstate::HEADERS;

        std::string transferencoding; //필드값 저장용, chunked 사용하는지 확인
        size_t contentLength = 0; //본문 길이 파악

        // headers 벡터를 파싱
        const char *header_start = data;
        while (header_start < message_end) {
            const char *line_end = strstr(header_start, "\r\n");
            if (line_end == nullptr) {
                // 완전한 헤더 라인이 아닌 경우, 기다림
                return 0;
            }
             // 헤더 라인 추출 및 처리
            const char *colon = strchr(header_start, ':');
            if (colon == nullptr || colon > line_end) {
                // 잘못된 헤더 형식, 이 오류를 처리합니다.
                Errorhandle(parser, HPE_INVALID_HEADER_TOKEN);
                return -1;
            }
            header_start = line_end + 2;

            
            // 헤더 필드와 값을 추출
            std::string header_field(header_start, colon - header_start);
            std::string header_value(colon + 1, line_end - colon - 1);
            
            header_field.erase(std::remove_if(header_field.begin(), header_field.end(), ::isspace), header_field.end());
            header_value.erase(std::remove_if(header_value.begin(), header_value.end(), ::isspace), header_value.end());

            if (!header_field.empty()) {
                http_on_Header_Field(header_field.c_str(), header_field.length());
                http_on_Header_Value(header_value.c_str(), header_value.length());
            }
            
            // Transfer-Encoding 헤더
            if (header_field == "Transfer-Encoding") {
                transferencoding = header_value;
            }
            // Content-Length 헤더
            if (header_field == "Content-Length") {
                contentLength = std::stoul(header_value);
            }

            // 헤더 파싱이 끝나면 바디 파싱
            parser->state = Httpparserstate::BODY;


            if(header_field == "Content-Length"){
                if(contentLength > 0) {
                    // contentLength 변수에 저장된 길이만큼 데이터를 bodydata에서 읽어와서 파싱
                    std::string body(body_mark, body_mark + contentLength);
                    http_on_Body(bodydata.c_str(), contentLength);
                }
                else {
                    // Content-Length 값이 0 미만인 경우 오류 처리
                    Errorhandle(parser, HPE_INVALID_CONTENT_LENGTH);
                    return -1;
                }
            }
            else if(header_field == "Transfer-Encoding"){
                if(transferencoding == "chunked") {
                    // chunked 인코딩을 사용하는 경우 각 청크를 파싱
                    size_t pos = 0;
                    while (pos < bodydata.size()) {
                        // 청크 크기를 파싱하고, 그 크기만큼 데이터를 읽기
                        size_t chunkSize = std::stoul(bodydata.substr(pos), nullptr, 16);
                        pos = bodydata.find("\r\n", pos) + 2;  // 청크 크기 뒤의 CRLF를 건너뛰기
                        if (chunkSize == 0){
                        http_on_Body(bodydata.c_str(), bodydata.length());
                        break;
                        }
                        std::string chunk = bodydata.substr(pos, chunkSize);
                        pos += chunkSize + 2; // 2는 청크 종료를 나타내는 \r\n
                        http_on_Body(chunk.c_str(), chunkSize); 
                        http_onMessageComplete();
                    }
                }
            }
      
            else {
            // Transfer-Encoding이 "chunked"가 아닌 값인 경우는 ..?
            }

    
        if ((parser->method == Http_method::GET ||
            parser->method == Http_method::HEAD ||
            parser->method == Http_method::DELETE ||
            parser->method == Http_method::OPTIONS) &&
            contentLength > 0) {
            Errorhandle(parser, HPE_INVALID_CONTENT_LENGTH); }
        //바디 없을때, 예를 들어 head나,,, 이런 method
      
        std::cout << "Body:" << std::endl;
        std::cout << bodydata << std::endl;
        
    } 
}
}

/*

#include <iostream>
#include <string>
#include "parser.h"
#define KEEP_ALIVE "keep-alive"
#define CLOSE "close"
#define CONTENT_LENGTH "content-length"
#define PROXY_CONNECTION "proxy-connection"
#define CONNECTION "connection"
#include <sstream>


class HttpRequest {
public:
    std::string method;
    std::string url;
    std::string HTTP_version;
    std::map<std::string, std::string> headers;
    std::string body;

    HttpRequest() {}

};

class HttpResponse {
public:
    std::string HTTP_version;
    std::string status_code;
    std::string status_message;
    std::map<std::string, std::string> headers;
    std::string body;

    HttpResponse() {}
};

class HttpParser {
public:
    HttpParser() : method(HttpMethod::UNDEFINED), state(HttpParserState::IDLE), nread(0) {}

    void init(HttpMethod method) {
        this->method = method;
        state = HttpParserState::IDLE;
        startLine.clear();
        headers.clear();
        bodydata.clear();
        nread = 0;
    }

    
*/
