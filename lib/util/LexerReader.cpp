#include "LexerReader.h"

LexerReader::LexerReader(std::string filePath)
{
    inputType = InputType::File;
    bData = nullptr;
    stream = new std::ifstream(filePath,std::ios::binary);
    if (!stream)
    {
        throwError("file read error");
    }
    else if (!stream->is_open())
    {
        delete stream;
        throwError("file read error");
    }
}

LexerReader::LexerReader(std::list<Util::byte> bData)
{
    inputType = InputType::BinaryData;
    stream = nullptr;
    if (bData.empty())
    {
        return;
    }
    this->bData = new std::list<Util::byte>(bData);
}

LexerReader::~LexerReader()
{
    if (stream)
    {
        stream->close();
        delete stream;
        stream = nullptr;
    }
    
    if (bData)
    {
        delete bData;
        bData = nullptr;
    }
}

std::tuple<Util::byte,bool> LexerReader::get()
{
    if (inputType == InputType::File)
    {
        if (stream && !stream->eof())
        {
            auto ch = stream->get();
            if (ch != EOF)
            {
                return std::make_tuple(ch,true);
            }
        }
    }
    else if (inputType == InputType::BinaryData)
    {
        if (bData && !bData->empty())
        {
            auto ch = bData->front();
            bData->pop_front();
            return std::make_tuple(ch,true);
        }
    }
    
    return std::make_tuple(EOF,false);
}
