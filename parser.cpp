
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

enum class Method
{
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    TRACE,
    OPTIONS,
    CONNECT,
    PATCH
};

string to_string(Method method)
{
    switch(method)
    {
        case Method::GET:
            return "GET";
        case Method::HEAD:
            return "HEAD";
        case Method::POST:
            return "POST";
        case Method::PUT:
            return "PUT";
        case Method::DELETE:
            return "DELETE";
        case Method::TRACE:
            return "TRACE";
        case Method::OPTIONS:
            return "OPTIONS";
        case Method::CONNECT:
            return "CONNECT";
        case Method::PATCH:
            return "PATCH";
    }
}

Method method_from_string (const string& method) noexcept
{
    if (method == to_string(Method::GET))
    {
        return Method::GET;
    }
    else if (method == to_string(Method::HEAD))
    {
        return Method::HEAD;
    }
    else if (method == to_string(Method::POST))
    {
        return Method::POST;
    }
    else if (method == to_string(Method::PUT))
    {
        return Method::PUT;
    }
    else if (method == to_string(Method::DELETE))
    {
        return Method::DELETE;
    }
    else if (method == to_string(Method::TRACE))
    {
        return Method::TRACE;
    }
    else if (method == to_string(Method::OPTIONS))
    {
        return Method::OPTIONS;
    }
    else if (method == to_string(Method::CONNECT))
    {
        return Method::CONNECT;
    }
    else if (method == to_string(Method::PATCH))
    {
        return Method::PATCH;
    }
}

class HTTPParser {
public:

    HTTPParser();

    void setHeaderCompleteCallback(void (*headerCompleteCallback)()) {
        headerCompleteCallback_ = headerCompleteCallback;
    }

    void setBodyCompleteCallback(void (*bodyCompleteCallback)()) {
        bodyCompleteCallback_ = bodyCompleteCallback;
    }


    vector<pair<string, string>> requestHeaders;
    vector<pair<string, string>> responseHeaders;

    string requestMethod;
    string requestURI;
    string requestHTTPVersion;

    string responseHTTPVersion;
    string responseStatus;
    string responseMessage;
    
    void parseMessage(const string& message);
    void parseRequest(const string& message);
    void parseResponse(const string& message);

    ~HTTPParser();

/*
private:
    // parse result
    http_request
    http_response
};
*/

private:
    void (*headerCompleteCallback_)() = nullptr;
    void (*bodyCompleteCallback_)() = nullptr;
    string startline;
    Method reqmethod;

    struct HttpRequest {
        string method;
        string uri;
        string http_version;
        vector<pair<string, string>> headers;
        string body;
    } http_request;

    struct HttpResponse {
        string http_version;
        string http_status;
        string http_message;
        vector<pair<string, string>> headers;
        string body;
    } http_response;

};

void HTTPParser::parseMessage(const string& message) {
    size_t startlineEndpos = message.find("\r\n");
    // startline 저장
    if (startlineEndpos != string::npos) {
        startline = message.substr(0, startlineEndpos);
    }
    cout << "startline : " << startline << endl;

    if (startline[0] == 'H'){
        if (startline[1]=='T'){
            parseResponse(message);}
        else
            parseRequest(message);}
    else
        parseResponse(message);

}
void HTTPParser::parseRequest(const string& message) {
    requestHeaders.clear();
    size_t startlineEndpos = message.find("\r\n");
    // startline 저장
    if (startlineEndpos != string::npos) {
        startline = message.substr(0, startlineEndpos);
    }
    else{
        cerr << "Invalid message format" << endl;
    }

    cout << "startline : " << startline << endl;
    // 멤버변수로 사용하기
    istringstream stream(startline);
    string Method, URI, HTTP_version;
    stream >> Method >> URI >> HTTP_version;
    reqmethod = method_from_string(Method);

    cout << "Method, URI, Http_version : " << Method << ", " << URI << ", " << HTTP_version << endl;

    size_t headerEndPos = message.find("\r\n\r\n"); // 헤더 끝나는 위치
    if (headerEndPos != string::npos) {
        string headerSection = message.substr(startlineEndpos + 2, headerEndPos);
        // 헤더 구간

        size_t headerstartPos = startlineEndpos+2; // 헤더 시작 위치
        while (headerstartPos != string::npos) {
            size_t headerlineend = headerSection.find("\r\n", headerstartPos);
            if (headerlineend != string::npos) {
                string headerLine = headerSection.substr(headerstartPos, headerlineend - headerstartPos);

                size_t colonPos = headerLine.find(": ");
                if (colonPos != string::npos) {
                    string key = headerLine.substr(0, colonPos);
                    string value = headerLine.substr(colonPos + 2);
                    requestHeaders.push_back(make_pair(key, value));
                }
                headerstartPos = headerlineend + 2;
            } else {
                headerstartPos = string::npos;
            }
        }
        if (headerCompleteCallback_) {
        headerCompleteCallback_();
    }

        cout << "Request Headers:" << endl;
        for (const auto& header : requestHeaders) {
            cout << header.first << ": " << header.second << endl;
        }
    }
    // 헤더 뽑아서 잘 나오나 확인해보고
    cout << "Request body parsing,,," << endl;
    string requestBody = message.substr(headerEndPos + 4);
    cout << "body : " << requestBody << endl;
    cout << "Request body parsing complete." << endl;

    http_request.method = to_string(reqmethod);
    http_request.uri = URI;
    http_request.http_version = HTTP_version;
    http_request.headers = requestHeaders;
    http_request.body = requestBody;

    }

// startline, header(content-type, content-length .. ), CRLF , body
void HTTPParser::parseResponse(const string& message) {
    responseHeaders.clear();
    
    size_t startlineEndpos = string(message).find("\r\n");
    // startline 저장
    
    if (startlineEndpos != string::npos){
        startline = message.substr(0,startlineEndpos);
    }
    else{
        cerr << "Invalid message format" << endl;
    }
    cout << "startline : " << startline << endl;
    istringstream stream(startline);
    string HTTP_version, Http_status, http_message;
    stream >> HTTP_version >> Http_status;
    getline(stream, http_message);

    cout << "http version, http_status, http_message : " << HTTP_version<< ", " << Http_status<< ", " << http_message << endl;

    size_t headerEndPos = message.find("\r\n\r\n"); // 헤더 끝나는 위치
    if (headerEndPos != string::npos) {
        string headerSection = message.substr(startlineEndpos + 2, headerEndPos - startlineEndpos - 2);
        // 헤더 구간

        size_t headerstartPos = startlineEndpos+2; // 헤더 시작 위치
        string contentType;  // Content-Type 헤더 값 저장
        int contentLength = -1; 

        while (headerstartPos != string::npos) {
            size_t headerlineend = headerSection.find("\r\n", headerstartPos);
            if (headerlineend != string::npos) {
                string headerLine = headerSection.substr(headerstartPos, headerlineend - headerstartPos);
                responseHeaders.push_back(make_pair(headerSection.substr(headerstartPos,headerlineend-headerstartPos),""));
                headerstartPos = headerlineend+2;
                
                size_t colonPos = headerLine.find(":");
                if (colonPos != string::npos) {
                    string key = headerLine.substr(0, colonPos);
                    string value = headerLine.substr(colonPos+1);
                    responseHeaders.push_back(make_pair(key, value));

                    if (key == "Content-Type"){
                        contentType = value;
                    }
                    else if (key=="Content-Length"){
                        contentLength = stoi(value);
                    } 
                }
                headerstartPos = headerlineend + 2;
            } else {
                headerstartPos = string::npos;
            }
        }
    if (headerCompleteCallback_) {
        headerCompleteCallback_();
    }

    cout << "Response Headers:" << endl;
    for (const auto& header : responseHeaders) {
        cout << header.first << ": " << header.second << endl;
    }

    cout << "Response body parsing..." << endl;

    string responseBody = string(message).substr(headerEndPos + 4);
    cout << "body : " << responseBody << endl;


    cout << "Response body parsing complete." << endl;
    
    http_response.http_version = HTTP_version;
    http_response.http_status = Http_status;
    http_response.http_message = http_message;
    http_response.headers = responseHeaders;
    http_response.body = responseBody;
    }

}
void onHeaderComplete() {
    cout << "Header processing complete. Triggering an action." << endl;
}

void onBodyComplete() {
    cout << "Body processing complete. Triggering a different action." << endl;
}

HTTPParser::HTTPParser() {
    cout << "HTTPParser new!" << endl;
}

HTTPParser::~HTTPParser() {
    cout << "HTTPParser dead!" << endl;
}



int main()
{
    HTTPParser parser;
    parser.setHeaderCompleteCallback(onHeaderComplete);
    parser.setBodyCompleteCallback(onBodyComplete);

    // HTTP 요청/응답 메시지 고정 
    string message = "GET /path HTTP/1.1\r\nHost: example.com\r\n\r\nThis is the request body.";
    parser.parseMessage(message);
    
    return 0;
}


/*
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

class HTTPParser {
public:

    HTTPParser();
    ~HTTPParser();

    void setHeaderCompleteCallback(void (*headerCompleteCallback)()) {
        headerCompleteCallback_ = headerCompleteCallback;
    }

    void setBodyCompleteCallback(void (*bodyCompleteCallback)()) {
        bodyCompleteCallback_ = bodyCompleteCallback;
    }


    vector<pair<string, string>> requestHeaders;
    vector<pair<string, string>> responseHeaders;

    string requestMethod;
    string requestURI;
    string requestHTTPVersion;
    //request일떄

    string responseHTTPVersion;
    string responseStatus;
    string responseMessage;
    //response일때
    
    
    void parseMessage(const string& message) {
        size_t startlineEndpos = message.find("\r\n");
        // startline 저장
        if (startlineEndpos != string::npos) {
            startline = message.substr(0, startlineEndpos);
        }
        cout << "startline : " << startline << endl;

        if (startline[0] == 'H'){
            if (startline[1]=='T'){
                parseResponse(message);}
            else
                parseRequest(message);}
        else
            parseResponse(message);

    }
    // startline, header(host, user-agent, accept ,, ), CRLF , body
    void parseRequest(const string& message) {
        requestHeaders.clear();
        size_t startlineEndpos = message.find("\r\n");
        // startline 저장
        if (startlineEndpos != string::npos) {
            startline = message.substr(0, startlineEndpos);
        }
        cout << "startline : " << startline << endl;
        // 멤버변수로 사용하기
        istringstream stream(startline);
        string Method, URI, HTTP_version;
        stream >> Method >> URI >> HTTP_version;

        cout << "Method, URI, Http_version : " << Method << ", " << URI << ", " << HTTP_version << endl;

        size_t headerEndPos = message.find("\r\n\r\n"); // 헤더 끝나는 위치
        if (headerEndPos != string::npos) {
            string headerSection = message.substr(startlineEndpos + 2, headerEndPos);
            // 헤더 구간

            size_t headerstartPos = startlineEndpos+2; // 헤더 시작 위치
            while (headerstartPos != string::npos) {
                size_t headerlineend = headerSection.find("\r\n", headerstartPos);
                if (headerlineend != string::npos) {
                    string headerLine = headerSection.substr(headerstartPos, headerlineend - headerstartPos);

                    size_t colonPos = headerLine.find(": ");
                    if (colonPos != string::npos) {
                        string key = headerLine.substr(0, colonPos);
                        string value = headerLine.substr(colonPos + 2);
                        requestHeaders.push_back(make_pair(key, value));
                    }
                    headerstartPos = headerlineend + 2;
                } else {
                    headerstartPos = string::npos;
                }
            }
            if (headerCompleteCallback_) {
            headerCompleteCallback_();
        }

            cout << "Request Headers:" << endl;
            for (const auto& header : requestHeaders) {
                cout << header.first << ": " << header.second << endl;
            }
        }
        // 헤더 뽑아서 잘 나오나 확인해보고
        cout << "Request body parsing,,," << endl;
        string requestBody = message.substr(headerEndPos + 4);
        cout << "body : " << requestBody << endl;
        cout << "Request body parsing complete." << endl;

    }

    // startline, header(content-type, content-length .. ), CRLF , body
    void parseResponse(const string& message) {
        responseHeaders.clear();
        
        size_t startlineEndpos = string(message).find("\r\n");
        // startline 저장
        
        if (startlineEndpos != string::npos){
            startline = message.substr(0,startlineEndpos);
        }
        cout << "startline : " << startline << endl;
        istringstream stream(startline);
        string HTTP_version, Http_status, http_message;
        stream >> HTTP_version >> Http_status;
        getline(stream, http_message);

        cout << "http version, http_status, http_message : " << HTTP_version<< ", " << Http_status<< ", " << http_message << endl;

        size_t headerEndPos = message.find("\r\n\r\n"); // 헤더 끝나는 위치
        if (headerEndPos != string::npos) {
            string headerSection = message.substr(startlineEndpos + 2, headerEndPos - startlineEndpos - 2);
            // 헤더 구간

            size_t headerstartPos = startlineEndpos+2; // 헤더 시작 위치
            string contentType;  // Content-Type 헤더 값 저장
            int contentLength = -1; 

            while (headerstartPos != string::npos) {
                size_t headerlineend = headerSection.find("\r\n", headerstartPos);
                if (headerlineend != string::npos) {
                    string headerLine = headerSection.substr(headerstartPos, headerlineend - headerstartPos);
                    responseHeaders.push_back(make_pair(headerSection.substr(headerstartPos,headerlineend-headerstartPos),""));
                    headerstartPos = headerlineend+2;
                    
                    size_t colonPos = headerLine.find(":");
                    if (colonPos != string::npos) {
                        string key = headerLine.substr(0, colonPos);
                        string value = headerLine.substr(colonPos+1);
                        responseHeaders.push_back(make_pair(key, value));

                        if (key == "Content-Type"){
                            contentType = value;
                        }
                        else if (key=="Content-Length"){
                            contentLength = stoi(value);
                        } 
                    }
                    headerstartPos = headerlineend + 2;
                } else {
                    headerstartPos = string::npos;
                }
            }
        if (headerCompleteCallback_) {
            headerCompleteCallback_();
        }

        cout << "Response Headers:" << endl;
        for (const auto& header : responseHeaders) {
            cout << header.first << ": " << header.second << endl;
        }

        cout << "Response body parsing..." << endl;

        string responseBody = string(message).substr(headerEndPos + 4);
        cout << "body : " << responseBody << endl;


        cout << "Response body parsing complete." << endl;
        }

    }

private:
    void (*headerCompleteCallback_)() = nullptr;
    void (*bodyCompleteCallback_)() = nullptr;
    string startline;

};

void onHeaderComplete() {
    std::cout << "Header processing complete. Triggering an action." << std::endl;
}

void onBodyComplete() {
    std::cout << "Body processing complete. Triggering a different action." << std::endl;
}

HTTPParser::HTTPParser() {
    cout << "HTTPParser new!" << endl;
}

HTTPParser::~HTTPParser() {
    cout << "HTTPParser dead!" << endl;
}



int main()
{
    HTTPParser parser;
    parser.setHeaderCompleteCallback(onHeaderComplete);
    parser.setBodyCompleteCallback(onBodyComplete);

    // HTTP 요청/응답 메시지 고정 
    string message = "GET /path HTTP/1.1\r\nHost: example.com\r\n\r\nThis is the request body.";
    // 파싱해볼 예시
    // httpRequest = "GET /path HTTP/1.1\r\nHost: example.com\r\n\r\nThis is the request body.";
    // httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nThis is the response body.";
    parser.parseMessage(message);
    
    return 0;
}



#include <cstring>
#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
#include "parser.h"
#define KEEP_ALIVE "keep-alive"
#define CLOSE "close"
#define CONTENT_LENGTH "content-length"
#define PROXY_CONNECTION "proxy-connection"
#define CONNECTION "connection"
#include <sstream>


#ifndef BIT_AT
# define BIT_AT(a, i)                                                \
  (!!((unsigned int) (a)[(unsigned int) (i) >> 3] &                  \
   (1 << ((unsigned int) (i) & 7))))
#endif



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

using namespace std;


class HttpParser
{
public:
    void http_on_Message_Begin();
    void http_startline_end(const string &startLine);
    void http_on_Url(const char *url, size_t length);
    void http_on_status(const char *status, size_t length);
    //void http_on_Header_Field(const char *field, size_t length);
    //void http_on_Header_Value( const char *value, size_t length);
    void http_on_Headers_Complete(vector<pair<string,string>>& headers);
    void http_on_Body( const char *body, size_t length);
    void http_onMessageComplete();
    Http_method parse_http_Method(const std::string& methodStr);
    void http_parser_init();
    int http_parser_execute(Httpparser *parser, HttpParser *settings, const char *data, size_t len);

    enum class HttpMessage{
        REQUEST,
        RESPONSE
    };
    HttpMessage message_type;
    
private:
    // HTTP 요청
    struct HttpRequest {
        string Method;
        string URI;
        string Http_version;
    }http_request;

    // HTTP 응답
    struct HttpResponse {
        string HTTP_version;
        string Http_status;
        string http_message;
    }http_response;
};



string MessageToString(HttpParser::HttpMessage message) {
    switch (message) {
        case HttpParser::HttpMessage::REQUEST:
            return "REQUEST";
        case HttpParser::HttpMessage::RESPONSE:
            return "RESPONSE";
        default:
            return "UNKNOWN";
    }
}

void HttpParser::http_on_Message_Begin(){
    cout << "Message Begin" << endl;
};

//// 4. 콜백으로 startline 끝났을 때 method, uri 들어가기
void HttpParser::http_startline_end(const string &startLine){

    if (message_type == HttpMessage::REQUEST) {
        istringstream stream(startLine);
        string Method, URI, HTTP_version;
        stream >> Method >> URI >> HTTP_version;
        // HttpRequest 구조체의 값들을 설정
        http_request.Method = Method;
        http_request.URI = URI;
        http_request.Http_version = HTTP_version;
    } else {
        istringstream stream(startLine);
        string HTTP_version, Http_status, http_message;
        stream >> HTTP_version >> Http_status;
        getline(stream, http_message);
        // HttpResponse 구조체의 값들을 설정
        http_response.HTTP_version = HTTP_version;
        http_response.Http_status = Http_status;
        http_response.http_message = http_message;
    }
};

/*

void HttpParser::http_on_Url(const char *url, size_t length){
    cout << "URL: " << string(url, length) << endl;
};

void HttpParser::http_on_status(const char *status, size_t length){
    cout << "status code: " << string(status, length) << endl;
};
*/

void HttpParser::http_on_Headers_Complete(vector<pair<string,string>>& headers){
    cout << "Headers Complete" << endl;
    for (const auto& header : headers) {
        const string& key = header.first;
        const string& value = header.second;
        cout << "Header Key: " << key << ", Value: " << value << endl;
    }
};

void HttpParser::http_on_Body( const char *body, size_t length){
   cout << "Body: " << string(body, length) << endl;
};

void HttpParser::http_onMessageComplete(){
    cout << "Message Complete" << endl;
};

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



// 1. 일단 다 클래스 형식으로 바꾸기
// 2. const char --> size_t , int 형식으로 바꿀 것
// 3. 헤더는 벡터에 key-value 형태로 push, body는 string
// 4. 콜백으로 startline 끝났을 때 method, uri 들어가기
// 5. 콜백 header_end 로 끝났을 때 key value 형태로 header 다 들어가게
// 6. body는 마지막 끝이 오거나 Content-Length or chunk 데이터로 끝 확인
// 7. 끝이 오면 data를 string에 저장하고 끝



//파서랑 메서드 초기화 때리고
void http_parser_init(Httpparser *parser, Http_method method){
    parser->state = Httpparserstate::IDLE;
    parser->method = method;
    parser->startLine.clear();
    parser->headers.clear();
    parser->bodydata.clear();
    parser->nread = 0; // nread 변수 초기화

};

Http_method parser_http_method(const std::string& method) {
    switch (method[0]) {
        case 'G':
            return Http_method::GET;
        case 'P':
            if (method == "POST") {
                return Http_method::POST;
            } else if (method == "PUT") {
                return Http_method::PUT;
            } else if (method == "PATCH") {
                return Http_method::PATCH;}
            break;
        case 'D':
            if (method == "DELETE") {
                return Http_method::DELETE;
            }
            break;
        case 'H':
            if (method == "HEAD") {
                return Http_method::HEAD;
            }
            break;
        case 'O':
            if (method == "OPTIONS") {
                return Http_method::OPTIONS;
            }
            break;
        case 'C':
            if (method == "CONNECT") {
                return Http_method::CONNECT;
            }
            break;
        case 'T':
            if (method == "TRACE") {
                return Http_method::TRACE;
            }
            break;
    }
    return Http_method::UNDEFINED;
}


// 메시지가 오면 startline, header, body 나누는 함수
void httpdivide(Httpparser& parser, const char *data, string& startLine, vector<string>& headers, string& bodydata) {
    // 빈 줄로 헤더와 본문을 분리
    size_t headerEnd = string(data).find("\r\n\r\n");
    // 다음이 두줄 띄어쓰기 있다? 그게 헤더의 끝

    if (headerEnd != string::npos) {
        // 시작 라인 추출
        size_t startLineEnd = string(data).find("\r\n");
        if (startLineEnd != string::npos) {
            startLine = string(data).substr(0, startLineEnd);
        }
        // 헤더 추출
        string headerSection = string(data).substr(startLineEnd + 2, headerEnd - startLineEnd - 2);
        size_t pos = 0; //헤더 필드 시작 위치
        while (pos != string::npos) {
            size_t lineEnd = headerSection.find("\r\n", pos);
            if (lineEnd != string::npos) {
                headers.push_back(headerSection.substr(pos, lineEnd - pos));
                pos = lineEnd + 2;
            } else {
                // 마지막 헤더
                headers.push_back(headerSection.substr(pos));
                pos = string::npos;
            }
        }
        // 본문 추출
        bodydata = string(data).substr(headerEnd + 4);
    }
}




int http_parser_execute(Httpparser *parser, HttpParser *settings, const char *data, size_t len){
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

    HttpParser httpparser;
    httpparser.http_on_Message_Begin();

    string startLine;
    vector<string> headers;
    string bodydata;

    httpdivide(*parser, data, startLine, headers, bodydata);
    //여기서 스타트라인-헤더-비디 가르고

    cout << "StartLine: " << startLine << endl;
        
    if (startLine[0] == 'H'){
        if (startLine[1]=='T'){
            httpparser.message_type = HttpParser::HttpMessage::RESPONSE;}
        else
            httpparser.message_type = HttpParser::HttpMessage::REQUEST;}
    else
        httpparser.message_type = HttpParser::HttpMessage::RESPONSE;
    //여기에서 startline 읽고 메시지 유형 결정

    cout << "Message Type: " << MessageToString(httpparser.message_type) << endl;

    httpparser.http_startline_end(startLine);
   
    header_field_mark = data + startLine.size() + 2;
    parser->state = Httpparserstate::HEADERS;

    std::string transferencoding;
    size_t contentLength = 0;

    size_t startlineEnd = startLine.find('\r\n');
    //스타트라인 끝난 위치

    const char *message_end = strstr(data, "\r\n\r\n");
    //message_end : 헤더가 끝난 위치 먼저 선언하고
    
    if (message_end != nullptr) {
        const char* body_mark = message_end + 4; // 헤더 끝난 자리 다음다음부터 바디 시작할
        parser->state = Httpparserstate::HEADERS;

        string transferencoding; //필드값 저장용, chunked 사용하는지 확인
        size_t contentLength = 0; //본문 길이 파악

        // headers 벡터를 파싱

        vector<pair<string,string>>headers;

        //const char *header_start = data + startLine.size() + 2;   
        size_t header_start = data + startLine.size() + 2 - data;

        while (header_start < message_end-data) {
            size_t line_end = strstr(data+header_start, "\r\n")-data;
            if (line_end == -1) {
                // 완전한 헤더 라인이 아닌 경우
                return 0;
            }
             // 헤더 라인 추출인데 
            size_t colon = strchr(data+header_start, ':')-data;
            if (colon == -1 || colon > line_end) {
                // 잘못된 헤더 형식->오류
                Errorhandle(parser, HPE_INVALID_HEADER_TOKEN);
                return -1;
            }
            header_start = line_end + 2;

            // 헤더 필드, 값 추출
            string header_field(data+header_start, colon-header_start);
            string header_value(data+colon + 1, line_end - colon - 1);


            //공백지워서
            header_field.erase(remove_if(header_field.begin(), header_field.end(), ::isspace), header_field.end());
            header_value.erase(remove_if(header_value.begin(), header_value.end(), ::isspace), header_value.end());
            
            // 3. 헤더는 벡터에 key-value 형태로 push, body는 string
            if (!header_field.empty()) {
                headers.push_back(make_pair(header_field, header_value));
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

            if ((parser->method == Http_method::GET ||
            parser->method == Http_method::HEAD ||
            parser->method == Http_method::DELETE ||
            parser->method == Http_method::OPTIONS) &&
            contentLength > 0) {
            Errorhandle(parser, HPE_INVALID_CONTENT_LENGTH); }
            //바디 없는 메서든데 contentLength가 0보다 크다고 할 때, 예를 들어 head나,,, 이런
      

            if (header_field == "Content-Length"){
                if(contentLength > 0 && body_mark + contentLength <= message_end) {
                    // contentLength 변수에 저장된 길이만큼 데이터를 bodydata에서 읽어와서 파싱
                    string bodydata(body_mark, body_mark + contentLength);
                    httpparser.http_on_Body(bodydata.c_str(), contentLength);}
                else {
                    // Content-Length 값이 0 미만인 경우 오류
                    Errorhandle(parser, HPE_INVALID_CONTENT_LENGTH);
                    return -1;}
            }
            else{
                if(transferencoding == "chunked"||transferencoding == "Chunked") {
                    // chunked 인코딩을 사용하는 경우 각 청크를 파싱
                    size_t pos = 0;
                    while (pos < bodydata.size()) {
                        // 청크 크기를 파싱하고, 그 크기만큼 데이터를 읽기
                        size_t chunkSize = stoul(bodydata.substr(pos), nullptr, 16);
                        pos = bodydata.find("\r\n", pos) + 2;  // 청크 크기 뒤의 CRLF를 건너뛰기
                        if (chunkSize == 0){
                            break;
                        }

                        if (pos + chunkSize <= bodydata.size()) {
                            string chunk = bodydata.substr(pos, chunkSize);
                            pos += chunkSize + 2; // 2는 청크 종료를 나타내는 \r\n
                            httpparser.http_on_Body(chunk.c_str(), chunkSize);
                      } else {
                          // 청크 크기가 실제 데이터보다 큰 경우 처리
                          break;
                        string chunk = bodydata.substr(pos, chunkSize);
                        pos += chunkSize + 2; // 2는 청크 종료를 나타내는 \r\n
                        httpparser.http_on_Body(chunk.c_str(), chunkSize); 
                        }
                    httpparser.http_onMessageComplete();
                }
                }
            }
        // 부족한 부분
        // 파싱함수에서 data에 남은 데이터 있는지 확인해야함
        size_t remain_data = len - (body_mark - data);
        if (remain_data > 0) {
            bodydata.append(body_mark, body_mark + remain_data);
        }
    } 
    }
}



int main() 
{
    string httpMessage =
    
    "HEAD /test/hi-there HTTP/1.1\r\n"
    "Host: www.example.com\r\n"
    "Content-Length: 15\r\n"
    "\r\n"
    "This is the body.\r\n"
    "Hello";
    /*
    "HTTP/1.1 200 OK\r\n"
    "Server: MyServer\r\n"
    "Transfer-Encoding: chunked\r\n"  // Transfer-Encoding 헤더
    "\r\n"
    "7\r\n"  
    "Hello, \r\n"
    "5\r\n" 
    "World!\r\n"
    "0\r\n" 
    "\r\n";
    */

    Httpparser parser;
    http_parser_init(&parser, Http_method::UNDEFINED);
    http_parser_execute(&parser, nullptr, httpMessage.c_str(), httpMessage.length());
    return 0;
}


#include <cstring>
#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
#include "parser.h"
#define KEEP_ALIVE "keep-alive"
#define CLOSE "close"
#define CONTENT_LENGTH "content-length"
#define PROXY_CONNECTION "proxy-connection"
#define CONNECTION "connection"
#include <sstream>


#ifndef BIT_AT
# define BIT_AT(a, i)                                                \
  (!!((unsigned int) (a)[(unsigned int) (i) >> 3] &                  \
   (1 << ((unsigned int) (i) & 7))))
#endif



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

using namespace std;


class HttpParser
{
public:
    void http_on_Message_Begin();
    void http_on_Url(const char *url, size_t length);
    void http_on_status(const char *status, size_t length);
    void http_on_Header_Field(const char *field, size_t length);
    void http_on_Header_Value( const char *value, size_t length);
    void http_on_Headers_Complete();
    void http_on_Body( const char *body, size_t length);
    void http_onMessageComplete();
    Http_method parse_http_Method(const std::string& methodStr);
    void http_parser_init();
    int http_parser_execute(Httpparser *parser, HttpParser *settings, const char *data, size_t len);
private:
    enum class HttpMessage{
        REQUEST,
        RESPONSE
    };
    HttpMessage message_type;

    // HTTP 요청
    struct HttpRequest {
        string Method;
        string URI;
        string Http_version;
        vector<string> headers;
        string Body;
    }http_request;

    // HTTP 응답
    struct HttpResponse {
        string HTTP_version;
        string Http_status;
        string http_message;
        vector<string> headers;
        string Body;
    }http_response;


};

void HttpParser::http_on_Message_Begin(){
    cout << "Message Begin" << endl;
};

void HttpParser::http_on_Url(const char *url, size_t length){
    cout << "URL: " << string(url, length) << endl;
};

void HttpParser::http_on_status(const char *status, size_t length){
    cout << "status code: " << string(status, length) << endl;
};

void HttpParser::http_on_Header_Field(const char *field, size_t length) {
    std::vector<std::string> header;
    header.emplace_back(field, length);  // field를 벡터에 추가
    cout << "Header Field: " << string(field, length) << endl;
};

void HttpParser::http_on_Header_Value( const char *value, size_t length){
   cout << "Header Value: " << string(value, length) << endl;
};

void HttpParser::http_on_Headers_Complete(){
    cout << "Headers Complete" << endl;
};

void HttpParser::http_on_Body( const char *body, size_t length){
   cout << "Body: " << string(body, length) << endl;
};

void HttpParser::http_onMessageComplete(){
    cout << "Message Complete" << endl;
};



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



// 1. 일단 다 클래스 형식으로 바꾸기
// 2. const char --> size_t , int 형식으로 바꿀 것 
// 3. 헤더는 벡터에 key-value 형태로 push, body는 string
// 4. 콜백으로 startline 끝났을 때 method, uri 들어가기
// 5. 콜백 header_end 로 끝났을 때 key value 형태로 header 다 들어가기
// 6. body는 마지막 끝이 오거나 Content-Length or chunk 데이터로 끝 확인
// 7.끝이 오면 data를 string에 저장하고 끝


HttpParser httpparser;

//파서랑 메서드 초기화 때리고
void http_parser_init(Httpparser *parser, Http_method method){
    parser->state = Httpparserstate::IDLE;
    parser->method = method;
    parser->startLine.clear();
    parser->headers.clear();
    parser->bodydata.clear();
    parser->nread = 0; // nread 변수 초기화

};

//여긴 switch 문으로 바꿔
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


Http_method parser_http_method(const std::string& method) {
    switch (method[0]) {
        case 'G':
            return Http_method::GET;
        case 'P':
            if (method == "POST") {
                return Http_method::POST;
            } else if (method == "PUT") {
                return Http_method::PUT;
            } else if (method == "PATCH") {
                return Http_method::PATCH;
            }
            break;
        case 'D':
            if (method == "DELETE") {
                return Http_method::DELETE;
            }
            break;
        case 'H':
            if (method == "HEAD") {
                return Http_method::HEAD;
            }
            break;
        case 'O':
            if (method == "OPTIONS") {
                return Http_method::OPTIONS;
            }
            break;
        case 'C':
            if (method == "CONNECT") {
                return Http_method::CONNECT;
            }
            break;
        case 'T':
            if (method == "TRACE") {
                return Http_method::TRACE;
            }
            break;
    }
    return Http_method::UNDEFINED;
}

// 메시지가 오면 startline, header, body 나누는 함수
void httpdivide(Httpparser& parser, const char *data, string& startLine, vector<string>& headers, string& bodydata) {
    // 빈 줄로 헤더와 본문을 분리
    size_t headerEnd = string(data).find("\r\n\r\n");
    // 다음이 두줄 띄어쓰기 있다? 그게 헤더의 끝

    if (headerEnd != string::npos) {
        // 시작 라인 추출
        size_t startLineEnd = string(data).find("\r\n");
        if (startLineEnd != string::npos) {
            startLine = string(data).substr(0, startLineEnd);
        }
        // 헤더 추출
        string headerSection = string(data).substr(startLineEnd + 2, headerEnd - startLineEnd - 2);
        size_t pos = 0; //헤더 필드 시작 위치
        while (pos != string::npos) {
            size_t lineEnd = headerSection.find("\r\n", pos);
            if (lineEnd != string::npos) {
                headers.push_back(headerSection.substr(pos, lineEnd - pos));
                pos = lineEnd + 2;
            } else {
                // 마지막 헤더
                headers.push_back(headerSection.substr(pos));
                pos = string::npos;
            }
        }
        // 본문 추출
        bodydata = string(data).substr(headerEnd + 4);
    }
}


int http_parser_execute(Httpparser *parser, HttpParser *settings, const char *data, size_t len){
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

    httpparser.http_on_Message_Begin();

    string startLine;
    vector<string> headers;
    string bodydata;

    httpdivide(*parser, data, startLine, headers, bodydata);
    //여기서 스타트라인-헤더-비디 가르고

    cout << "StartLine: " << startLine << endl;

    string message_type;
        
    if (startLine[0] == 'H'){
        if (startLine[1]=='T'){
            message_type = "response";}
        else
            message_type = "request";}
    else
        message_type = "response";
    //여기에서 startline 읽고 메시지 유형 결정

    cout << "Message Type: " << message_type << endl;

    if (message_type == "request") {
        istringstream stream(startLine);
        string Method, URI, HTTP_version;
        stream >> Method >> URI >> HTTP_version;

        parser->method = parser_http_method(Method);

        url_mark = data + std::string(data).find(URI);
        size_t length = URI.length();
        httpparser.http_on_Url(URI.c_str(), length);
        cout << "HTTP_version: " << HTTP_version;

        header_field_mark = data + startLine.size() + 2;
        parser->state = Httpparserstate::HEADERS;
      } 

    else {
        istringstream stream(startLine);
        string HTTP_version, Http_status, http_message;
        stream >> HTTP_version >> Http_status;
        getline(stream,http_message);

        status_mark = data+startLine.find(Http_status);        
        httpparser.http_on_status(Http_status.c_str(), Http_status.length());
        cout << "http message: " << http_message << endl; 

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

        string transferencoding; //필드값 저장용, chunked 사용하는지 확인
        size_t contentLength = 0; //본문 길이 파악

        // headers 벡터를 파싱
        const char *header_start = data + startLine.size() + 2;   
        while (header_start < message_end) {
            const char *line_end = strstr(header_start, "\r\n");
            if (line_end == nullptr) {
                // 완전한 헤더 라인이 아닌 경우
                return 0;
            }
             // 헤더 라인 추출인데 
            const char *colon = strchr(header_start, ':');
            if (colon == nullptr || colon > line_end) {
                // 잘못된 헤더 형식->오류
                Errorhandle(parser, HPE_INVALID_HEADER_TOKEN);
                return -1;
            }
            header_start = line_end + 2;

            
            // 헤더 필드, 값 추출
            string header_field(header_start, header_start-colon);
            string header_value(colon + 1, line_end - colon - 1);

            //공백지워서
            header_field.erase(remove_if(header_field.begin(), header_field.end(), ::isspace), header_field.end());
            header_value.erase(remove_if(header_value.begin(), header_value.end(), ::isspace), header_value.end());

            if (!header_field.empty()) {
                httpparser.http_on_Header_Field(header_field.c_str(), header_field.length());
                httpparser.http_on_Header_Value(header_value.c_str(), header_value.length());
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

            if ((parser->method == Http_method::GET ||
            parser->method == Http_method::HEAD ||
            parser->method == Http_method::DELETE ||
            parser->method == Http_method::OPTIONS) &&
            contentLength > 0) {
            Errorhandle(parser, HPE_INVALID_CONTENT_LENGTH); }
            //바디 없는 메서든데 contentLength가 0보다 크다고 할 때, 예를 들어 head나,,, 이런
      

            if (header_field == "Content-Length"){
                if(contentLength > 0) {
                    // contentLength 변수에 저장된 길이만큼 데이터를 bodydata에서 읽어와서 파싱
                    string body(body_mark, body_mark + contentLength);
                    httpparser.http_on_Body(bodydata.c_str(), contentLength);
                }
                else {
                    // Content-Length 값이 0 미만인 경우 오류
                    Errorhandle(parser, HPE_INVALID_CONTENT_LENGTH);
                    return -1;
                }
            }

              
            else
            {
                if(transferencoding == "chunked"||transferencoding == "Chunked") {
                    // chunked 인코딩을 사용하는 경우 각 청크를 파싱
                    size_t pos = 0;
                    while (pos < bodydata.size()) {
                        // 청크 크기를 파싱하고, 그 크기만큼 데이터를 읽기
                        size_t chunkSize = stoul(bodydata.substr(pos), nullptr, 16);
                        pos = bodydata.find("\r\n", pos) + 2;  // 청크 크기 뒤의 CRLF를 건너뛰기
                        if (chunkSize == 0){
                            break;
                        }

                        if (pos + chunkSize <= bodydata.size()) {
                            string chunk = bodydata.substr(pos, chunkSize);
                            pos += chunkSize + 2; // 2는 청크 종료를 나타내는 \r\n
                            httpparser.http_on_Body(chunk.c_str(), chunkSize);
                      } else {
                          // 청크 크기가 실제 데이터보다 큰 경우 처리
                          break;
                        string chunk = bodydata.substr(pos, chunkSize);
                        pos += chunkSize + 2; // 2는 청크 종료를 나타내는 \r\n
                        httpparser.http_on_Body(chunk.c_str(), chunkSize); 
                        }
                    httpparser.http_onMessageComplete();

                }
               
                else {
                // Transfer-Encoding이 "chunked"가 아닌 값인 경우는 ..?
                  
                }
               
                }
            }

        // 부족한 부분
        // 파싱함수에서 data에 남은 데이터 있는지 확인해야함
        //
        
    } 
    }
}



int main() 
{
    string httpMessage =
    
    "HEAD /test/hi-there HTTP/1.1\r\n"
    "Host: www.example.com\r\n"
    "Content-Length: 15\r\n"
    "\r\n"
    "This is the body.\r\n"
    "Hello";
   
    "HTTP/1.1 200 OK\r\n"
    "Server: MyServer\r\n"
    "Transfer-Encoding: chunked\r\n"  // Transfer-Encoding 헤더
    "\r\n"
    "7\r\n"  
    "Hello, \r\n"
    "5\r\n" 
    "World!\r\n"
    "0\r\n" 
    "\r\n";
    

    Httpparser parser;
    http_parser_init(&parser, Http_method::UNDEFINED);
    http_parser_execute(&parser, nullptr, httpMessage.c_str(), httpMessage.length());
    return 0;
}

*/
