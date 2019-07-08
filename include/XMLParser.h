#ifndef XML_PARSER_H
#define XML_PARSER_H

#include "XMLLexer.h"
#include "XMLDocument.h"
#include <stack>
#include <deque>
#include <memory>

class XMLParser
{
public:
    XMLParser(std::string filePath,bool _isHTML = false);
    XMLParser(std::list<Util::byte> bData,bool _isHTML = false);
    ~XMLParser();
    
    std::unique_ptr<XMLDocument> xmlTextToDocument();
private:
    std::stack<XMLTok *> tokenStack;
    std::stack<XMLDocument *> elementStack;
    XMLLex *lex;
    bool isHTML;
    std::deque<XMLTok *> *htmlTokQueue;
private:
    XMLTok * getNextToken();
    
    void parse_tag_declare();
    
    void parse_cdata();
    
    void parse_content();
    
    void parse_tag_end();
    
    void getAllToken();
    bool fixNoneSelfClosedTag(std::string name);
};

#endif
