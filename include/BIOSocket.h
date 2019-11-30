#ifndef BIO_SOCKET_H
#define BIO_SOCKET_H

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <vector>
#include <string>
#include "UtilDef.h"
#include <sys/socket.h>

class BIOSocket 
{
public:
    BIOSocket(bool _isViaSSL = false);
    ~BIOSocket();

    bool connect(std::string host,std::string port);
    ssize_t send(void *buffer,size_t len);
    std::vector<Util::byte> receive(int recvBufSize = 512);
    bool close();

    bool setSocketOpt(int item, int opt, const void *val, socklen_t len);
    bool setBlocking(bool block);
private:
    SSL *ssl;
    SSL_CTX *sslCtx;
    BIO *socketBIO;

    bool isViaSSL;
};

#endif