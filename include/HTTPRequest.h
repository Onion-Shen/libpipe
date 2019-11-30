#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <map>
#include <ostream>

class HTTPRequest
{
public:
    HTTPRequest();
    ~HTTPRequest();
    
    void initParameter();
    
    std::string toRequestMessage();
    void addRequestHeader(std::string key,std::string value);
    void setRequestLine(std::string _method,std::string _path,std::string _version);
public:
    std::string method;
    std::string path;
    std::string version;
    
    std::map<std::string,std::string> *header;
    
    std::basic_string<unsigned char> body;
};

std::ostream & operator << (std::ostream &os,HTTPRequest *res);

std::ostream & operator << (std::ostream &os,HTTPRequest& res);

#endif
