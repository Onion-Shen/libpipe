#ifndef URL_H
#define URL_H

#include <string>
#include <ostream>
#include <map>

class URL
{
public:
    URL(std::string str);
    URL(const URL &url);

    friend std::ostream & operator<<(std::ostream &os, const URL &url);
    friend std::ostream & operator<<(std::ostream &os, const URL *url);

    std::map<std::string,std::string> queryDic();

public:
    std::string originalURL;
    std::string scheme;
    std::string host;
    int portNumber;
    std::string path;
    std::string query;
private:
    void parseURLStr(std::string urlStr);
    std::string urlEncode(std::string &str);
};

#endif
