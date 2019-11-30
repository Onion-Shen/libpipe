#ifndef HTTP_RECV_MSG_PARSER_H
#define HTTP_RECV_MSG_PARSER_H

#include <deque>
#include <memory>
#include "HTTPResponse.h"
#include "UtilDef.h"

class HTTPRecvMsgParser
{
public:
    HTTPRecvMsgParser();
    ~HTTPRecvMsgParser();

    bool is_parse_msg();
    std::shared_ptr<HTTPResponse> msg2res();

    void addToCache(std::vector<Util::byte> &bytes);
private:
    bool parse_line();
    bool parse_header();
    bool parse_body();
public:
    std::string version;
    std::string status_code;
    std::string reason;

    std::map<std::string, std::string> header;

    std::string method;
private:
    std::deque<Util::byte> *cache;
    HTTPMessageParseState state;
    long content_length;
    bool isChunk;
};

#endif
