#ifndef WS_SERVER_H
#define WS_SERVER_H

#include "UtilDef.h"
#include "TCPSocket.h"
#include <string>
#include <functional>
#include <unordered_map>

enum class WSClientMsgType
{
    None = 0,
    Text,
    Bolb
};

struct WSClientMsg
{
public:
    std::string msg;
    WSClientMsgType type;
    bool connect;

    WSClientMsg() : type(WSClientMsgType::None), connect(true)
    {
        msg = std::string();
    }
};

class WSServer
{
public:
    WSServer(int port) : port(port) 
    {
        setSocket();
        clients = std::unordered_map<int, std::shared_ptr<TCPSocket>>();
    }

public:
    void sendMsg(std::string msg, std::shared_ptr<TCPSocket> fd) const;
    void loop();

public:
    std::function<void(const std::shared_ptr<TCPSocket> &)> connect;
    std::function<void(const std::shared_ptr<TCPSocket> &)> close;
    std::function<void(const std::shared_ptr<TCPSocket> &, std::string)> error;
    std::function<void(const std::shared_ptr<TCPSocket> &, WSClientMsg &)> receive;

private:
    void setSocket();
    WSClientMsg parseRecvData(std::vector<Util::byte> bytes);
    std::shared_ptr<TCPSocket> handShaking();

private:
    std::shared_ptr<TCPSocket> sock;
    std::unordered_map<int, std::shared_ptr<TCPSocket>> clients;
    int port;
};

#endif
