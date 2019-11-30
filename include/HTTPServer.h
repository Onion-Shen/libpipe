#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "HTTPReqMsgParser.h"
#include "TCPSocket.h"

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__)
#define Kqueue
#elif defined(__linux__)
#define Epoll
#endif

using LoopCallback = std::function<void(HTTPRequest &request, HTTPResponse &response)>;

class HTTPServer
{
  public:
    HTTPServer(int port) : port(port)
    {
        dict = new std::map<int, std::shared_ptr<TCPSocket>>();
    }

    ~HTTPServer();

    void loop(LoopCallback callback);

  private:
    int port;
    TCPSocket *sock = nullptr;
    int listenfd = -1;
    std::map<int, std::shared_ptr<TCPSocket>> *dict = nullptr;

  private:
    void setSocket();
    void disconnect(int sockfd);

#ifdef Kqueue
    void kqueueLoop(const LoopCallback &callback);
    int setupKqueue();
    void kqueueAccept(long count, int kq);
    void deleteEvent(int sockfd, int kq, int eventType);
    int kqueueParseRecvRequest(HTTPRequest &request, HTTPReqMsgParser &parser, long totalLength, int sockfd);
#elif defined(Epoll)
    void epollLoop(const LoopCallback &callback);
#endif
};

#endif
