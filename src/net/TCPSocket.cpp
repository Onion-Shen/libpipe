#include "TCPSocket.h"
#include "UtilDef.h"
#include <vector>
#include <unistd.h>

bool TCPSocket::listen(int backlog)
{
    return ::listen(sockfd(), backlog) == 0;
}

bool TCPSocket::connect(std::string address, int port)
{
    if (!sockaddrinfo)
    {
        initSockaddr(address, port);

        if (!sockaddrinfo)
        {
            throwError("address is null");
            return false;
        }
    }

    auto addr = reinterpret_cast<sockaddr *>(sockaddrinfo);
    auto len = SocketConfig::addressLen(*sockaddrinfo);
    return ::connect(sockfd(), addr, len) == 0;
}

ssize_t TCPSocket::send(void *buffer, size_t len)
{
    return ::write(sockfd(), buffer, len);
}

ssize_t TCPSocket::receive(std::vector<Util::byte> &ref)
{
    Util::byte tmpBuf[recvBuffSize];
    auto bytes = recv(sockfd(), tmpBuf, recvBuffSize, 0);
    if (bytes > 0)
    {
        ref.insert(std::end(ref), tmpBuf, tmpBuf + bytes);
    }
    return bytes;
}

std::vector<Util::byte> TCPSocket::receive()
{
    Util::byte tmpBuf[recvBuffSize];
    auto bytes = recv(sockfd(), tmpBuf, recvBuffSize, 0);

    if (bytes == -1)
    {
        throwError("recv error  " + std::string(gai_strerror(errno)));
        return std::vector<Util::byte>();
    }

    return std::vector<Util::byte>(tmpBuf, tmpBuf + bytes);
}

std::shared_ptr<TCPSocket> TCPSocket::accept()
{
    auto client = (sockaddr_storage *)malloc(sizeof(sockaddr_storage));
    socklen_t len = sizeof(sockaddr_storage);

    auto clientfd = ::accept(sockfd(), reinterpret_cast<sockaddr *>(client), &len);
    if (clientfd == -1)
    {
        throwError("accept error " + std::string(gai_strerror(errno)));
        return nullptr;
    }
    
    return std::make_shared<TCPSocket>(clientfd, client, SocketType::TCP);
}