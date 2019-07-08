#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <memory>
#include "JSONLexer.h"
#include "JSObject.h"

class JSONParser
{
public:
    JSONParser(std::string filePath);
    JSONParser(std::list<Util::byte> binaryData);
public:
    std::shared_ptr<JSObject> tokenToJSObject();
private:
    std::shared_ptr<JSObject>   tokenToPrimitive(std::shared_ptr<JSONToken> &token);
    std::shared_ptr<JSArray>    tokenToJSArray();
    std::shared_ptr<JSMap>      tokenToJSMap();
    
    std::shared_ptr<JSObject> parserMainLoop(std::shared_ptr<JSONToken> &token);
private:
    std::shared_ptr<JSONLexer> lexer;
};

#endif
