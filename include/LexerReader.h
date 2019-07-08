#ifndef LEXER_READER_H
#define LEXER_READER_H

#include "UtilDef.h"
#include <string>
#include <fstream>
#include <tuple>
#include <list>

enum class InputType : int
{
    File = 0,
    BinaryData
};

class LexerReader
{
private:
    InputType inputType;
    std::ifstream *stream;
    std::list<Util::byte> *bData;
public:
    LexerReader(std::string filePath);
    LexerReader(std::list<Util::byte> bData);
    
    ~LexerReader();
public:
    std::tuple<Util::byte,bool> get();
};

#endif
