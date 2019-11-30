#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include "URL.h"
#include "HTTPResponse.h"
#include "HTTPRequest.h"
#include "TCPSocket.h"
#include "BIOSocket.h"

enum class HTTPMethod
{
    GET = 0,
    POST,
    HEAD
};
    
class HTTPClient
{
public:
    void setRequestHeader(std::string key,std::string value);
    
    std::shared_ptr<HTTPResponse> request();

    //default is 10
    long timeoutSeconds = 10;
public:
    HTTPClient(std::string _url, HTTPMethod _method = HTTPMethod::GET) : method(_method)
    {
        url = new URL(std::move(_url));
        isViaSSL = url->scheme == "https";
        setHttpRequest();
    }

    ~HTTPClient();
private:
    void setHttpRequest();
    std::string methodStr();

    std::shared_ptr<HTTPResponse> requestByTCPSocket();
    std::string setSocketParams();
    void setSocketOptions();
    void sendMsg();
    std::shared_ptr<HTTPResponse> recvMsg();
    void closeConnection();

    std::shared_ptr<HTTPResponse> requestViaSSL();
    
private:
    HTTPMethod method;
    URL *url = nullptr;
    HTTPRequest *httpRequest = nullptr;

    TCPSocket *socket = nullptr;
    bool isConnect = false;

    BIOSocket *socketViaSSL = nullptr;
    bool isViaSSL = false;
};

#endif
