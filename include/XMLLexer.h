#ifndef XML_LEXER_H
#define XML_LEXER_H

#include "UtilDef.h"
#include "XMLTok.h"
#include "LexerReader.h"

class XMLLex
{
public:
    XMLLex(std::string filePath);
    XMLLex(std::list<Util::byte> bData);
    
    XMLTok * getNextTok();
private:
    std::shared_ptr<LexerReader> reader;
    TokType state;
    TokType lastTokType;
    std::string localCache;
private:
    std::tuple<Util::byte,bool> getNextChar();
    
    void initState(int16_t ch);
    void tagStartState(int16_t ch);
    XMLTok * commentState(int16_t ch);
    XMLTok * fileAttributeState(int16_t ch);
    XMLTok * tagEndState(int16_t ch);
    XMLTok * tagDeclareState(int16_t ch);
    XMLTok * contentState(int16_t ch);
    XMLTok * docTypeState(int16_t ch);
    XMLTok * CDataState(int16_t ch);
};

#endif
