#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include "BasicSocket.h"
#include "UtilDef.h"

class UDPSocket : public BasicSocket
{
public:
    UDPSocket(int domain, AddressFamily family) : BasicSocket(domain, SocketType::UDP, family)
    {
        if (sockfd() == -1)
        {
            throwError("invalid socket fd");
        }
    }

    using RecvObjType = std::tuple<std::vector<Util::byte>, sockaddr_storage>;
    RecvObjType recvFrom();

    ssize_t sendto(void *buffer, size_t len, sockaddr_storage &addr);
};

#endif