#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include "BasicSocket.h"
#include "UtilDef.h"
#include <memory>
#include <vector>
#include <cerrno>

class TCPSocket : public BasicSocket
{
  public:
    TCPSocket(int domain, AddressFamily family) : BasicSocket(domain, SocketType::TCP, family)
    {
        if (sockfd() == -1)
        {
            throwError("invalid socket fd");
        }
    }

    TCPSocket(int sockfd, sockaddr_storage *addr, SocketType _socktype) : BasicSocket(sockfd, addr, _socktype) {}

  public:
    bool listen(int backlog = 10);

    bool connect(std::string address, int port);

    ssize_t send(void *buffer, size_t len);

    std::vector<Util::byte> receive();
    ssize_t receive(std::vector<Util::byte> &ref);

    std::shared_ptr<TCPSocket> accept();

    template <typename BufType>
    ssize_t write(BufType buf, size_t size)
    {
        size_t written = 0;
        size_t left = size;
        while (left > 0)
        {
            ssize_t byte_size = send(&buf[written], left);
            if (byte_size > 0)
            {
                auto convert_size = static_cast<size_t>(byte_size);
                left -= convert_size;
                written += convert_size;
            }
            else
            {
                if (written < 0 && errno == EINTR)
                {
                    //call write again
                    written = 0;
                }
                else
                {
                    //error occur
                    return -1;
                }
            }
        }
        return static_cast<ssize_t>(size);
    }
};

#endif