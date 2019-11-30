#include "HTTPServer.h"

#ifdef Kqueue

constexpr int MaxEventCount = 10;

#include <sys/event.h>
#include <vector>
#include "STLExtern.h"

#elif defined(Epoll)
#include <sys/epoll.h>
#endif

#ifdef Util_Debug
#include <iostream>
#endif

HTTPServer::~HTTPServer()
{
    if (sock)
    {
        delete sock;
        sock = nullptr;
    }

    if (dict)
    {
        STLExtern::releaseMap(dict);
        delete dict;
        dict = nullptr;
    }
}

void HTTPServer::disconnect(int sockfd)
{
    auto iter = dict->find(sockfd);
    if (iter == dict->end())
    {
        return;
    }

    auto client = iter->second;
    dict->erase(sockfd);
    if (client != nullptr)
    {
        client.reset();
    }
}

void HTTPServer::setSocket()
{
    auto address = std::string();
    auto family = AddressFamily::IPV4;

    auto addressList = SocketConfig::getAddressInfo(address, std::to_string(port), family, SocketType::TCP);
    if (!addressList)
    {
        throwError("getAddressInfo() return null");
        return;
    }

    auto domain = addressList->ai_family;
    freeaddrinfo(addressList);
    addressList = nullptr;

    sock = new TCPSocket(domain, family);

    int yes = 1;
    if (!sock->setSocketOpt(SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)))
    {
        throwError("set SO_REUSEADDR flag error");
        return;
    }

    if (!sock->bind(address, port))
    {
        throwError("bind() error " + std::string(gai_strerror(errno)));
        return;
    }

    if (!sock->listen())
    {
        throwError("listen() error " + std::string(gai_strerror(errno)));
    }

    listenfd = sock->sockfd();
    if (!SocketConfig::setNonBlocking(listenfd))
    {
        throwError("set none blocking error : " + std::string(gai_strerror(errno)));
    }

#ifdef __APPLE__
    yes = 1;
    if (!sock->setSocketOpt(SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof(yes)))
    {
        throwError("set SO_NOSIGPIPE flag error");
        return;
    }
#endif
}

void HTTPServer::loop(LoopCallback callback)
{
    if (!callback)
    {
        return;
    }

    setSocket();

#ifdef Kqueue
    kqueueLoop(callback);
#elif defined(Epoll)
    epollLoop(callback);
#endif
}

#pragma mark-- Kqueue
#ifdef Kqueue
int HTTPServer::setupKqueue()
{
    auto kq = kqueue();
    if (kq == -1)
    {
        throwError("kqueue() error : " + std::string(gai_strerror(errno)));
    }

    struct kevent listenEvent;
    EV_SET(&listenEvent, listenfd, EVFILT_READ, EV_ADD, 0, 0, nullptr);
    if (kevent(kq, &listenEvent, 1, nullptr, 0, nullptr) == -1)
    {
        throwError("kevent() error : " + std::string(gai_strerror(errno)));
    }

    return kq;
}

void HTTPServer::kqueueAccept(long count, int kq)
{
    for (auto i = 0; i < count; ++i)
    {
        auto client = sock->accept();
        if (!client)
        {
            continue;
        }

        auto clientfd = client->sockfd();
        if (clientfd <= 0)
        {
            continue;
        }

        if (!SocketConfig::setNonBlocking(clientfd))
        {
            throwError("set none blocking error : " + std::string(gai_strerror(errno)));
        }

        struct kevent changelist[2];
        EV_SET(&changelist[0], clientfd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);
        EV_SET(&changelist[1], clientfd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, nullptr);
        kevent(kq, changelist, 2, nullptr, 0, nullptr);

        dict->insert(std::make_pair(clientfd, client));
    }
}

void HTTPServer::deleteEvent(int sockfd, int kq, int eventType)
{
    struct kevent event;
    EV_SET(&event, sockfd, eventType, EV_DELETE, 0, 0, nullptr);
    kevent(kq, &event, 1, nullptr, 0, nullptr);
}

//0 : normal 1 : empty 2 : error
int HTTPServer::kqueueParseRecvRequest(HTTPRequest &request, HTTPReqMsgParser &parser, long totalLength, int sockfd)
{
    int status = 0;

    auto iter = dict->find(sockfd);
    if (iter == dict->end() || iter->second == nullptr)
    {
        return 2;
    }
    auto client = iter->second;

    request.initParameter();
    parser.initParams();

    long recvBytes = 0;
    auto recvBuf = std::vector<Util::byte>();
    while (recvBytes <= totalLength)
    {
        auto size = totalLength - recvBytes;
        if (size > 0)
        {
            sock->recvBuffSize = size;
        }

        try
        {
            auto buff = client->receive();
            recvBuf.insert(recvBuf.end(), buff.begin(), buff.end());
        }
        catch (std::logic_error error)
        {
#ifdef Util_Debug
            std::cerr << error.what() << std::endl;
#endif

            status = 2;
            parser.msg2req(request);
            break;
        }

        if (recvBuf.empty())
        {
            status = 1;
            parser.msg2req(request);
            break;
        }

        recvBytes += recvBuf.size();
        parser.addToCache(recvBuf);
        recvBuf.clear();

        if (parser.is_parse_msg())
        {
            parser.msg2req(request);
            break;
        }
    }

    return status;
}

void HTTPServer::kqueueLoop(const LoopCallback &callback)
{
    auto kq = setupKqueue();
    auto request = HTTPRequest();
    auto response = HTTPResponse();
    auto parser = HTTPReqMsgParser();
    struct kevent eventlist[MaxEventCount];

    while (true)
    {
        auto activeEventCount = kevent(kq, nullptr, 0, eventlist, MaxEventCount, nullptr);

        for (auto i = 0; i < activeEventCount; i++)
        {
            auto event = eventlist[i];
            auto sockfd = static_cast<int>(event.ident);

            if (event.flags & EV_ERROR || sockfd <= 0) //error event
            {
                deleteEvent(sockfd, kq, event.filter);

                if (i + 1 == activeEventCount)
                {
                    disconnect(sockfd);
                }

                continue;
            }

            if (sockfd == listenfd) //new client
            {
                kqueueAccept(event.data, kq);
                continue;
            }

            if (event.filter == EVFILT_READ) //read event
            {
                auto status = kqueueParseRecvRequest(request, parser, event.data, sockfd);
                if (status == 0)
                {
                    response.initParameter();
                    callback(request, response);
                }
                else if (status == 1 || status == 2)
                {
                    deleteEvent(sockfd, kq, EVFILT_READ);
                    if (i + 1 == activeEventCount)
                    {
                        deleteEvent(sockfd, kq, EVFILT_WRITE);
                        disconnect(sockfd);
                    }
                }
            }
            else if (event.filter == EVFILT_WRITE) //write event
            {
                auto iter = dict->find(sockfd);
                if (iter == dict->end() || iter->second == nullptr)
                {
                    deleteEvent(sockfd, kq, EVFILT_WRITE);
                    if (i + 1 == activeEventCount)
                    {
                        disconnect(sockfd);
                    }
                    continue;
                }

                try
                {
                    auto client = iter->second;
                    auto message = response.toResponseMessage();
                    client->write(message, message.size());
                }
                catch (std::logic_error error)
                {
                    auto code = errno;
                    if (code != 32)
                    {
                        deleteEvent(sockfd, kq, EVFILT_READ);
                        deleteEvent(sockfd, kq, EVFILT_WRITE);
                        disconnect(sockfd);
                    }
                    continue;
                }

                if (request.header->find("Connection")->second == "Close")
                {
                    deleteEvent(sockfd, kq, EVFILT_READ);
                    deleteEvent(sockfd, kq, EVFILT_WRITE);
                    disconnect(sockfd);
                    break;
                }
            }
        }
    }
}
#endif

#pragma mark-- Epoll
#ifdef Epoll
void HTTPServer::epollLoop(const LoopCallback &callback)
{
    #error plan to rewrite it by TCPSocket 
}
#endif
