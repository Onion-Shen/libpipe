#include "HTTPRequest.h"
#include "UtilDef.h"
#include "Strings.h"

HTTPRequest::HTTPRequest()
{
    method = std::string();
    path = std::string();
    version = std::string();

    header = nullptr;

    body = std::basic_string<Util::byte>();
}

HTTPRequest::~HTTPRequest()
{
    if (header)
    {
        header->clear();
        delete header;
        header = nullptr;
    }
}

void HTTPRequest::initParameter()
{
    method.clear();
    path.clear();
    version.clear();

    if (header)
    {
        header->clear();
    }

    body.clear();
}

void HTTPRequest::addRequestHeader(std::string key,std::string value)
{
    if (!key.empty() && !value.empty())
    {
        if (!header)
        {
            header = new std::map<std::string,std::string>();
        }
        header->insert({key,value});
    }
}

void HTTPRequest::setRequestLine(std::string _method,std::string _path,std::string _version)
{
    method = std::move(_method);
    path = std::move(_path);
    version = std::move(_version);
}

std::string HTTPRequest::toRequestMessage()
{
    auto requestLine = Strings::join({method, path, version}, std::string(" "));

    auto headerStr = std::string();
    if (header && !header->empty())
    {
        for (auto pair : *header)
        {
            headerStr += pair.first + ": " + pair.second + "\r\n";
        }
    }

    auto requestBody = std::string(body.begin(), body.end());
    return Strings::join({requestLine, headerStr, requestBody}, std::string("\r\n"));
}

std::ostream & operator << (std::ostream &os,HTTPRequest *res)
{
    os<<*res;
    return os;
}

std::ostream & operator << (std::ostream &os,HTTPRequest& res)
{
    os<<res.toRequestMessage();
    return os;
}
