#ifndef HTTP_REQ_MSG_PARSER_H
#define HTTP_REQ_MSG_PARSER_H

#include <deque>
#include "HTTPRequest.h"
#include "UtilDef.h"

class HTTPReqMsgParser
{
public:
    HTTPReqMsgParser() = default;
    ~HTTPReqMsgParser();
    
    bool is_parse_msg();
    
    void msg2req(HTTPRequest &req);
    void initParams();

    void addToCache(std::vector<Util::byte> &bytes);
public:
    std::string method = std::string();
    std::string path = std::string();
    std::string version = std::string();
    
    std::map<std::string,std::string> header = std::map<std::string, std::string>();
private:
    std::deque<Util::byte> *cache = nullptr;
    HTTPMessageParseState state = HTTPMessageParseState::Line;
    long content_length = -1;
private:
    bool parse_line();
    bool parse_header();
    bool parse_body();
};

#endif
