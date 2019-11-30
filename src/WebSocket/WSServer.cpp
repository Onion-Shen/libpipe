#include "WSServer.h"
#include <sys/select.h>
#include <sys/errno.h>
#include <algorithm>
#include <vector>
#include "HTTPReqMsgParser.h"
#include "HTTPResponse.h"
#include "Crypto.h"

void WSServer::setSocket()
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

    sock = std::make_shared<TCPSocket>(domain, family);

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
}

WSClientMsg WSServer::parseRecvData(std::vector<Util::byte> bytes)
{
    auto msg = WSClientMsg();

    if (bytes.empty())
    {
        return msg;
    }

    /*
    %x0：   表示一个延续帧。当Opcode为0时，表示本次数据传输采用了数据分片，当前收到的数据帧为其中一个数据分片。
    %x1：   表示这是一个文本帧（frame）
    %x2：   表示这是一个二进制帧（frame）
    %x3-7： 保留的操作代码，用于后续定义的非控制帧。
    %x8：   表示连接断开。
    %x9：   表示这是一个ping操作。
    %xA：   表示这是一个pong操作。
    %xB-F： 保留的操作代码，用于后续定义的控制帧。
    */
    auto opcode = bytes[0] & 15;

    if (opcode == 0x8)
    {
        msg.connect = false;
        return msg;
    }

    if (opcode == 0x1)
    {
        msg.type = WSClientMsgType::Text;
    }
    else if (opcode == 0x2)
    {
        msg.type = WSClientMsgType::Bolb;
    }

    auto len = bytes.size();

    auto data = std::vector<Util::byte>(); //数据
    auto mask = std::vector<Util::byte>(); //掩码(4位)

    //第二个字节的后7位表示数据的长度
    auto code_len = bytes[1] & 127;

    if (code_len == 126)
    {
        //等于126，用后面相邻的2个字节来保存一个64bit位的无符号整数作为数据的长度,mask从第5个字节(4)开始
        for (auto i = 4; i < len; ++i)
        {
            if (i >= 8)
            {
                data.push_back(bytes[i]);
            }
            else
            {
                mask.push_back(bytes[i]);
            }
        }
    }
    else if (code_len > 126)
    {
        //大于126，用后面相邻的8个字节来保存一个64bit位的无符号整数作为数据的长度,mask从第11个字节(10)开始
        for (auto i = 10; i < len; ++i)
        {
            if (i >= 14)
            {
                data.push_back(bytes[i]);
            }
            else
            {
                mask.push_back(bytes[i]);
            }
        }
    }
    else
    {
        //小于126时playload只有1位，所以mask从第三位(2)开始
        for (auto i = 2; i < len; ++i)
        {
            if (i >= 6)
            {
                data.push_back(bytes[i]);
            }
            else
            {
                mask.push_back(bytes[i]);
            }
        }
    }

    auto dataSize = data.size();
    for (auto idx = 0; idx < dataSize; ++idx)
    {
        char code = data[idx] ^ mask[idx % 4];
        msg.msg += code;
    }

    return msg;
}

void WSServer::sendMsg(std::string msg, std::shared_ptr<TCPSocket> fd) const
{
    if (msg.empty() || !fd)
    {
        return;
    }

    auto bytes = std::vector<Util::byte>();
    bytes.push_back(0x81);

    auto length = msg.size();
    if (length < 126)
    {
        bytes.push_back(length);
    }
    else if (length <= 0xFFFF)
    {
        bytes.push_back(126); //126不到1个字节，不用转

        auto num = htons(length); //要转成网络字节序
        bytes.push_back(num & 0xFF);
        bytes.push_back((num & 0xFF00) >> 8);
    }
    else
    {
        bytes.push_back(127); //127不到1个字节，不用转

        auto num = htons(length); //要转成网络字节序
        bytes.push_back(num & 0xFF);
        bytes.push_back((num & 0xFF00) >> 8);
    }

    bytes.insert(std::end(bytes), std::begin(msg), std::end(msg));
    fd->write(bytes.data(), bytes.size()); 
}

void WSServer::loop()
{
    auto read_fds = fd_set();
    FD_ZERO(&read_fds);

    auto master = fd_set();
    FD_ZERO(&master);

    const auto listener = sock->sockfd();
    FD_SET(listener, &master);

    auto fd_max = listener;

    while (true)
    {
        read_fds = master;

        if (select(fd_max + 1, &read_fds, nullptr, nullptr, nullptr) == -1)
        {
            throwError("select error " + std::string(gai_strerror(errno)));
            return;
        }

        for (auto i = 0; i <= fd_max; ++i)
        {
            if (!FD_ISSET(i, &read_fds))
            {
                continue;
            }

            if (i == listener)
            {
                auto clientfd = handShaking();
                if (clientfd)
                {
                    clients[clientfd->sockfd()] = clientfd;

                    FD_SET(clientfd->sockfd(), &master);
                    fd_max = std::max(clientfd->sockfd(), fd_max);
                    if (connect)
                    {
                        connect(clientfd);
                    }
                }
                continue;
            }

            auto iter = clients.find(i);
            if (iter == std::end(clients))
            {
                continue;
            }

            auto client = iter->second;
            while (true)
            {
                auto recvBuf = std::vector<Util::byte>();
                auto bytes = client->receive(recvBuf);

                if (bytes == -1)
                {
                    //an error occurred
                    auto errmsg = gai_strerror(errno);
                    auto errormessage = std::string();
                    if (errmsg)
                    {
                        errormessage += errmsg;
                        delete errmsg;
                    }

                    if (error)
                    {
                        error(client, errormessage);
                    }
                    clients.erase(i);
                    FD_CLR(i, &master);
                    break;
                }
                else if (bytes == 0)
                {
                    //the peer has closed its half side of the connection.
                    if (close)
                    {
                        close(client);
                    }
                    clients.erase(i);
                    FD_CLR(i, &master);
                    break;
                }
                else if (bytes > 0)
                {
                    auto msg = parseRecvData(recvBuf);
                    if (msg.connect)
                    {
                        if (receive)
                        {
                            receive(client, msg);
                        }
                    }
                    else
                    {
                        if (close)
                        {
                            close(client);
                        }
                        clients.erase(i);
                        FD_CLR(i, &master);
                        break;
                    }
                }
            }
        }
    }
}

std::shared_ptr<TCPSocket> WSServer::handShaking()
{
    auto fd = sock->accept();
    if (!fd)
    {
        return nullptr;
    }

    auto parser = HTTPReqMsgParser();
    while (true)
    {
        auto recvbuf = fd->receive();
        if (recvbuf.empty())
        {
            break;
        }

        parser.addToCache(recvbuf);
        if (parser.is_parse_msg())
        {
            break;
        }
    }

    auto key = parser.header["Sec-WebSocket-Key"];
    if (key.empty())
    {
        return nullptr;
    }

    key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    auto sha1_str = Crypto::sha1encode(reinterpret_cast<Util::byte *>(const_cast<char *>(key.c_str())), key.size());
    if (sha1_str.empty())
    {
        return nullptr;
    }

    auto u8_str = reinterpret_cast<char *>(const_cast<Util::byte *>(sha1_str.c_str()));
    auto b64_str = Crypto::b64encode(u8_str);
    if (b64_str.empty())
    {
        return nullptr;
    }

    auto response = HTTPResponse();
    response.setResponseLine("HTTP/1.1", 101, "Switching Protocols");
    response.addResponseHead("Upgrade", "websocket");
    response.addResponseHead("Connection", "Upgrade");
    response.addResponseHead("Sec-WebSocket-Accept", b64_str);

    auto msg = response.toResponseMessage();
    fd->write(msg, msg.size());

    return fd;
}
