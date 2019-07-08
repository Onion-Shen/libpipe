#ifndef JSON_LEXER_h
#define JSON_LEXER_h

#include <deque>
#include <memory>
#include "JSONToken.h"
#include "UtilDef.h"
#include "LexerReader.h"

enum class LexerState
{
    Init = 0,
    Number,
    String,
    Bool,
    Null
};

class JSONLexer
{
public:
    JSONLexer(std::string filePath);
    JSONLexer(std::list<Util::byte> _binaryData);

    ~JSONLexer();
public:
    std::shared_ptr<JSONToken> getNextToken();
private:
    void initParams();
    std::tuple<Util::byte,bool> nextChar();
    
    std::shared_ptr<JSONToken> initState(char ch);
    std::shared_ptr<JSONToken> numberState(char ch);
    std::shared_ptr<JSONToken> stringState(char ch);
    std::shared_ptr<JSONToken> booleanState(char ch);
    std::shared_ptr<JSONToken> nullState(char ch);
    
    bool isInteger(std::string &content);
    bool isFloat(std::string &content);
private:
    LexerState state;
    std::deque<Util::byte> *cache;
    std::shared_ptr<LexerReader> reader;
};

#endif 
