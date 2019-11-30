#include "HTTPRecvMsgParser.h"
#include <cctype>
#include <algorithm>
#include "ZlibWrapper.h"
#include "Strings.h"

HTTPRecvMsgParser::~HTTPRecvMsgParser()
{
    if (cache)
    {
        cache->clear();
        delete cache;
        cache = nullptr;
    }
}

HTTPRecvMsgParser::HTTPRecvMsgParser()
{
    state = HTTPMessageParseState::Line;
    cache = nullptr;
    version = std::string();
    status_code = std::string();
    reason = std::string();
    header = std::map<std::string,std::string>();
    content_length = -1;
    isChunk = false;
    method = std::string();
}

void HTTPRecvMsgParser::addToCache(std::vector<Util::byte> &bytes)
{
    if (!bytes.empty())
    {
        if (!cache)
        {
            cache = new std::deque<Util::byte>();
        }
        cache->insert(cache->end(), std::begin(bytes), std::end(bytes));
    }
}

bool HTTPRecvMsgParser::is_parse_msg()
{   
    if (!cache || cache->empty())
    {
        return state != HTTPMessageParseState::Line;
    }
    
    auto res = false;
    if (state == HTTPMessageParseState::Line)
    {
        if (parse_line())
        {
            state = HTTPMessageParseState::Header;
            if (!cache->empty())
            {
                goto Header;
            }
        }
    }
    else if (state == HTTPMessageParseState::Header)
    {
    Header:
        if (parse_header())
        {
            if (method == "HEAD")
            {
                res = true;
            }
            else 
            {
                state = HTTPMessageParseState::Body;
                if (!cache->empty())
                {
                    goto Body;
                }
            }
        }
    }
    else if (state == HTTPMessageParseState::Body)
    {
    Body:
        res = parse_body();
    }
    
    return res;
}

bool HTTPRecvMsgParser::parse_body()
{
    auto res = false;
    
    if (content_length != -1)
    {
        long size = cache->size();
        if (size >= content_length)
        {
            res = true;
        }
    }
    else if (isChunk)
    {
        auto idx = -1;
        auto bytes = {Util::byte('\r'),Util::byte('\n'),Util::byte('0'),Util::byte('\r'),Util::byte('\n'),Util::byte('\r'),Util::byte('\n')};
        auto ite = std::search(cache->begin(),cache->end(),bytes.begin(),bytes.end());
        if (ite != cache->end())
        {
            idx = ite - cache->begin();
        }
        
        if (idx != -1)
        {
            auto count = cache->size() - idx;
            while (count > 0)
            {
                cache->pop_back();
                count--;
            }
            
            for (decltype(cache->size()) i = 0;i < cache->size() - 1;++i)
            {
                if (cache->at(i)     == Util::byte('\r') &&
                    cache->at(i + 1) == Util::byte('\n'))
                {
                    auto isHex = true;
                    auto hexStr = std::string(cache->begin(),cache->begin() + i);
                    for (auto ch : hexStr)
                    {
                        if (!isxdigit(ch))
                        {
                            isHex = false;
                        }
                    }
                    
                    if (isHex)
                    {
                        auto j = i + 2;
                        while (j > 0)
                        {
                            cache->pop_front();
                            j--;
                        }
                    }
                    
                    break;
                }
            }
            
            res = true;
        }
    }
    
    return res;
}

bool HTTPRecvMsgParser::parse_header()
{
    std::initializer_list<Util::byte> bytes;
    if (method == "HEAD")
    {
        bytes = {Util::byte('\r'), Util::byte('\n')};
    }
    else
    {
        bytes = {Util::byte('\r'), Util::byte('\n'), Util::byte('\r'), Util::byte('\n')};
    }
    auto ite = std::search(cache->begin(), cache->end(), bytes.begin(), bytes.end());

    if (ite == cache->end())
    {
        return false;
    }

    auto idx = ite - cache->begin();

    if (idx != -1)
    {
        auto key = std::string();
        auto value = std::string();
        auto flag = 0;
        
        for (auto i = 0;i < idx;++i)
        {
            auto ch = cache->at(0);
            cache->pop_front();
            
            if (ch           == Util::byte('\r') &&
                cache->at(0) == Util::byte('\n'))
            {
                header[key] = value;
                
                key.clear();
                value.clear();
                
                flag = 0;
                
                i++;
                cache->pop_front();
                continue;
            }
            
            if (flag == 0)
            {
                if (ch           == Util::byte(':') &&
                    cache->at(0) == Util::byte(' '))
                {
                    flag = 1;
                    i++;
                    cache->pop_front();
                }
                else
                {
                    key += ch;
                }
            }
            else if (flag == 1)
            {
                value += ch;
            }
        }
        
        //pop \r\n\r\n
        cache->pop_front();
        cache->pop_front();
        cache->pop_front();
        cache->pop_front();
        
        auto ite = header.find("Content-Length");
        if (ite != header.end())
        {
            content_length = atol(ite->second.c_str());
        }
        else
        {
            ite = header.find("Transfer-Encoding");
            if (ite != header.end())
            {
                isChunk = ite->second == "chunked";
            }
        }
    }
    
    return true;
}

bool HTTPRecvMsgParser::parse_line()
{
    auto buffer = std::string();
    auto LF = std::string();
    
    while (!cache->empty())
    {
        auto ch = cache->at(0);
        cache->pop_front();

        if (ch == Util::byte('\r') || ch == Util::byte('\n'))
        {
            LF += ch;
            if (LF.size() == 2)
            {
                if (LF == "\r\n")
                {
                    break;
                }
                else
                {
                    throwError("invalid http response line format");
                    return false;
                }
            }
        }
        else
        {
            buffer += ch;
        }
    }

    if (LF != "\r\n" || buffer.empty())
    {
        throwError("error occur in parse line");
        return false;
    }

    auto lineComponents = Strings::split(buffer, std::string(" "));
    if (lineComponents.size() != 3)
    {
        throwError("invalid http response line format");
        return false;
    }

    version = std::move(lineComponents[0]);
    status_code = std::move(lineComponents[1]);
    reason = std::move(lineComponents[2]);

    return true;
}

std::shared_ptr<HTTPResponse> HTTPRecvMsgParser::msg2res()
{
    auto response = std::make_shared<HTTPResponse>();

    response->setResponseLine(version, atoi(status_code.c_str()), reason);
    response->header = new std::map<std::string, std::string>(std::move(header));

    auto bufferLen = cache->size();
    if (bufferLen > 0)
    {
        Util::byte buffer[bufferLen];
        std::copy(cache->begin(), cache->end(), buffer);

        Util::byte *ptr = nullptr;
        size_t size = 0;

        if (header["Content-Encoding"] == "gzip")
        {
            uLong uncompressedBufferLen = bufferLen * 9;
            Byte uncompressedBuffer[uncompressedBufferLen];
            if (Zlib::gzlib_uncompress(buffer, bufferLen, uncompressedBuffer, &uncompressedBufferLen) == 0)
            {
                ptr = uncompressedBuffer;
                size = uncompressedBufferLen;
            }
        }
        else
        {
            ptr = buffer;
            size = bufferLen;
        }

        if (ptr && size > 0)
        {
            auto body = std::basic_string<Util::byte>(ptr, size);
            response->body = std::move(body);
        }
    }

    return response;
}