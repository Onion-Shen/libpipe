#include "HTTPClient.h"
#include "HTTPRecvMsgParser.h"

#ifdef Util_Debug
#include <cerrno>
#include <cstdio>
#endif

HTTPClient::~HTTPClient()
{
    if (url)
    {
        delete url;
        url = nullptr;
    }

    if (httpRequest)
    {
        delete httpRequest;
        httpRequest = nullptr;
    }

    closeConnection();
}

void HTTPClient::setRequestHeader(std::string key, std::string value)
{
    httpRequest->addRequestHeader(key, value);
}

std::shared_ptr<HTTPResponse> HTTPClient::request()
{
    if (isViaSSL) 
    {
        return requestViaSSL();
    }
    return requestByTCPSocket();
}

void HTTPClient::setHttpRequest()
{
    if (!url || url->path.empty())
    {
        throwError("url is null");
        return;
    }

    if (!httpRequest)
    {
        httpRequest = new HTTPRequest();
    }

    //=== line ===
    httpRequest->method = methodStr();

    httpRequest->path = url->path;
    if (!url->query.empty())
    {
        httpRequest->path += "?" + url->query;
    }

    httpRequest->version = "HTTP/1.1";

    //=== header ===
    if (url->host.empty())
    {
        throwError("url's host is null");
    }
    setRequestHeader("Host", url->host);
}

std::string HTTPClient::methodStr()
{
    if (method == HTTPMethod::GET)
    {
        return "GET";
    }
    else if (method == HTTPMethod::POST)
    {
        return "POST";
    }
    else if (method == HTTPMethod::HEAD)
    {
        return "HEAD";
    }

    return std::string();
}

#pragma mark -- http request via ssl
std::shared_ptr<HTTPResponse> HTTPClient::requestViaSSL()
{
    if (!socketViaSSL) 
    {
        socketViaSSL = new BIOSocket(true);
    }

    if (!isConnect) 
    {
        isConnect = socketViaSSL->connect(url->host,std::to_string(url->portNumber));
    }

    try
    {
        sendMsg();
    }
    catch (const std::exception &e)
    {
#ifdef Util_Debug
        std::cerr << e.what() << std::endl;
#endif
        isConnect = false;
        return nullptr;
    }

    auto response = recvMsg();

    if (isConnect)
    {
        auto isKeepAlive = false;
        if (response != nullptr && response->header && !response->header->empty())
        {
            auto iter = response->header->find("Connection");
            if (iter != response->header->end() && iter->second == "Keep-Alive")
            {
                isKeepAlive = true;
            }
        }

        if (!isKeepAlive)
        {
            isConnect = false;
            closeConnection();
        }
    }

    return response;
}

#pragma mark -- http request
std::shared_ptr<HTTPResponse> HTTPClient::requestByTCPSocket()
{
    auto address = setSocketParams();
    if (address == "-1")
    {
        throwError("ip address is invalid");
        return nullptr;
    }
    if (!socket)
    {
        throwError("socket is null");
        return nullptr;
    }

    if (!isConnect)
    {
        auto port = url->portNumber;
        if (!socket->connect(address, port))
        {
#ifdef Util_Debug
            perror("connect() error : ");
#endif
            return nullptr;
        }
        isConnect = true;
    }

    try
    {
        sendMsg();
    }
    catch (const std::exception &e)
    {
#ifdef Util_Debug
        std::cerr << e.what() << std::endl;
#endif
        isConnect = false;
        return nullptr;
    }

    auto response = recvMsg();

    if (isConnect)
    {
        auto isKeepAlive = false;
        if (response != nullptr && response->header && !response->header->empty())
        {
            auto iter = response->header->find("Connection");
            if (iter != response->header->end() && iter->second == "Keep-Alive")
            {
                isKeepAlive = true;
            }
        }

        if (!isKeepAlive)
        {
            isConnect = false;
            closeConnection();
        }
    }

    return response;
}

std::string HTTPClient::setSocketParams()
{
    auto address = std::string();

    if (!socket)
    {
        auto host = url->host;
        auto family = AddressFamily::IPV4;
        auto port = std::to_string(url->portNumber);

        auto addressList = SocketConfig::getAddressInfo(host, port, family, SocketType::TCP);
        if (!addressList)
        {
            throwError("getAddressInfo() return null");
            return "-1";
        }

        auto domain = addressList->ai_family;
        address = SocketConfig::netAddressToHostAddress(*(addressList->ai_addr));
        freeaddrinfo(addressList);
        addressList = nullptr;

        socket = new TCPSocket(domain, family);

        setSocketOptions();
    }

    return address;
}

void HTTPClient::setSocketOptions()
{
    if (!socket)
    {
        throwError("socket is null");
        return;
    }

    //recv buff size
    int recvBufSize = 1024 * 10;
    socket->recvBuffSize = recvBufSize;
    if (!socket->setSocketOpt(SOL_SOCKET, SO_RCVBUF, &recvBufSize, sizeof(recvBufSize)))
    {
#ifdef Util_Debug
        perror("set recv buff error : ");
#endif
        return;
    }

    //timeout
    struct timeval timeout = {.tv_sec = timeoutSeconds, .tv_usec = 0};
    if (!socket->setSocketOpt(SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)))
    {
#ifdef Util_Debug
        perror("set timeout error : ");
#endif
    }
}

void HTTPClient::sendMsg()
{
    auto clientMsg = httpRequest->toRequestMessage();
    if (clientMsg.empty())
    {
        throwError("http request message is null");
        return;
    }

    auto bytes = const_cast<char *>(clientMsg.c_str());
    auto size = clientMsg.size();

    if (isViaSSL) 
    {
        SocketConfig::sendAll(socketViaSSL,bytes,size);
    }
    else 
    {
        SocketConfig::sendAll(socket,bytes,size);
    }
}

std::shared_ptr<HTTPResponse> HTTPClient::recvMsg()
{
    std::shared_ptr<HTTPResponse> response = nullptr;

    auto parser = HTTPRecvMsgParser();
    parser.method = methodStr();

    while (1)
    {
        auto recvbuf = isViaSSL ? socketViaSSL->receive() : socket->receive();
        if (recvbuf.empty())
        {
            response = parser.msg2res();
            isConnect = false;
            closeConnection();
            break;
        }

        parser.addToCache(recvbuf);
        if (parser.is_parse_msg())
        {
            response = parser.msg2res();
            break;
        }
    }

    return response;
}

void HTTPClient::closeConnection()
{
    if (socket)
    {
        delete socket;
        socket = nullptr;
    }

    if (socketViaSSL)
    {
        delete socketViaSSL;
        socketViaSSL = nullptr;
    }
}
