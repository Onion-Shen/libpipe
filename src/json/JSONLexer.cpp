#include "JSONLexer.h"
#include "STLExtern.h"
#include <cctype>
#include <cstring>

JSONLexer::JSONLexer(std::string filePath)
{
    initParams();
    reader = std::make_shared<LexerReader>(filePath);
}

JSONLexer::JSONLexer(std::list<Util::byte> _binaryData)
{
    initParams();
    reader = std::make_shared<LexerReader>(_binaryData);
}

void JSONLexer::initParams()
{
    state = LexerState::Init;
    cache = new std::deque<Util::byte>();
}

JSONLexer::~JSONLexer()
{
    if (cache)
    {
        delete cache;
    }
}

std::tuple<Util::byte,bool> JSONLexer::nextChar()
{
    if (cache && !cache->empty())
    {
        auto ch = cache->at(0);
        cache->pop_front();
        return std::make_tuple(ch,true);
    }
    
    return reader->get();
}

std::shared_ptr<JSONToken> JSONLexer::getNextToken()
{
    std::shared_ptr<JSONToken> token = nullptr;
    
    while (!token)
    {
        auto result = nextChar();
        if (std::get<1>(result) == false)
        {
            break;
        }
        
        auto ch = std::get<0>(result);
        switch (state) 
        {
            case LexerState::Init:
                token = initState(ch);
                break;
            case LexerState::Number:
                token = numberState(ch);
                state = LexerState::Init;
                break;
            case LexerState::String:
                token = stringState(ch);
                state = LexerState::Init;
                break;
            case LexerState::Bool:
                token = booleanState(ch);
                state = LexerState::Init;
                break;
            case LexerState::Null:
                token = nullState(ch);
                state = LexerState::Init;
                break;
            default:
                break;
        }
    }
    
    return token;
}

std::shared_ptr<JSONToken> JSONLexer::initState(char ch)
{
    std::shared_ptr<JSONToken> token = nullptr;
    
    if (ch == '"')
    {
        state = LexerState::String;
    }
    else if (ch == 't' || ch == 'f')
    {
        state = LexerState::Bool;
        cache->push_back(ch);
    }
    else if (ch == '[')
    {
        token = std::make_shared<JSONToken>(TokenType::LeftBracket,"[");
    }
    else if (ch == ']')
    {
        token = std::make_shared<JSONToken>(TokenType::RightBracket,"]");
    }
    else if (ch == '{')
    {
        token = std::make_shared<JSONToken>(TokenType::LeftBrace,"{");
    }
    else if (ch == '}')
    {
        token = std::make_shared<JSONToken>(TokenType::RightBrace,"}");
    }
    else if (ch == ':')
    {
        token = std::make_shared<JSONToken>(TokenType::Colon,":");
    }
    else if (isdigit(ch))
    {
        state = LexerState::Number;
        cache->push_back(ch);
    }
    else if (ch == 'n')
    {
        state = LexerState::Null;
    }
    else if (ch == '-')
    {
        auto result = nextChar();
        auto _ch = std::get<0>(result);
        if (!std::get<1>(result) || !isdigit(_ch))
        {
            throwError("unexcept input behind -");
            return token;
        }
        
        cache->push_back(ch);
        cache->push_back(_ch);
        state = LexerState::Number;
    }
    else if (ch == ',')
    {
        token = std::make_shared<JSONToken>(TokenType::Comma,",");
    }
    
    return token;
}

std::shared_ptr<JSONToken> JSONLexer::numberState(char ch)
{
    auto numberStr = std::string();
    if (isdigit(ch))
    {
        numberStr += ch;
    }
    
    while (1)
    {
        auto result = nextChar();
        if (!std::get<1>(result))
        {
            break;
        }
        
        auto next_char = std::get<0>(result);
        if (isdigit(next_char) || next_char == '.')
        {
            numberStr += next_char;
        }
        else
        {
            cache->push_back(next_char);
            break;
        }
    }
    
    if (numberStr.empty())
    {
        return nullptr;
    }
    
    auto type = TokenType::Null;
    
    if (isInteger(numberStr))
    {
        type = TokenType::Integer;
    }
    else if (isFloat(numberStr))
    {
        type = TokenType::Float;
    }
    
    if (type == TokenType::Null)
    {
        throwError("except a number type's input");
        return nullptr;
    }
    
    if (ch == '-')
    {
        numberStr = ch + numberStr;
    }
    
    return std::make_shared<JSONToken>(type, numberStr);
}

bool JSONLexer::isInteger(std::string &content)
{
    auto size = content.size();
    if (size == 0)
    {
        return false;
    }
    
    auto firstChar = content[0];
    if (size == 1)
    {
        return isdigit(firstChar);
    }
    
    if (!(firstChar >= '1' && firstChar <= '9'))
    {
        return false;
    }
    
    auto result = true;
    for (auto i = 1; i < size; ++i)
    {
        if (!isdigit(content[i]))
        {
            result = false;
            break;
        }
    }
    
    return result;
}

bool JSONLexer::isFloat(std::string &content)
{
    auto size = content.size();
    if (size < 3)
    {
        return false;
    }
    
    auto index = content.rfind(".");
    if (index == std::string::npos)
    {
        return false;
    }
    
    auto integer = content.substr(0, index);
    if (!isInteger(integer))
    {
        return false;
    }
    
    auto decimal = content.substr(index + 1, size - index);
    auto result = true;
    
    for (auto ch : decimal)
    {
        if (!isdigit(ch))
        {
            result = false;
            break;
        }
    }
    
    return result;
}

std::shared_ptr<JSONToken> JSONLexer::stringState(char ch)
{
    if (ch == '"')
    {
        return std::make_shared<JSONToken>(TokenType::String, "");
    }
    
    auto str = std::string();
    str += ch;
    
    while (1)
    {
        auto result = nextChar();
        if (!std::get<1>(result))
        {
            throwError("except \" at the end of string");
            return nullptr;
        }
        
        auto u8_ch = std::get<0>(result);
        if (u8_ch == '"')
        {
            //is escape character
            auto count = 0;
            int i = static_cast<int>(str.length() - 1);
            while (i >= 0)
            {
                auto r_ch = str[i];
                if (r_ch != '\\')
                {
                    break;
                }
                count++;
                i--;
            }

            if (count % 2 == 0)
            {
                break;
            }
        }

        str += u8_ch;
    }
    
    return std::make_shared<JSONToken>(TokenType::String, str);
}

std::shared_ptr<JSONToken> JSONLexer::booleanState(char ch)
{
    auto length = (ch == 't') ? 3 : ((ch == 'f') ? 4 : 0);
    if (length == 0)
    {
        throwError("except 't' or 'f' at the boolean state");
        return nullptr;
    }
    
    auto boolVal = std::string();
    boolVal += ch;
    
    for (auto i = 0; i < length; ++i)
    {
        auto result = nextChar();
        if (!std::get<1>(result))
        {
            break;
        }
        boolVal += std::get<0>(result);
    }
    
    if (boolVal != "true" && boolVal != "false")
    {
        throwError("invalid boolean literal");
        return nullptr;
    }
    
    return std::make_shared<JSONToken>(TokenType::Boolean, std::move(boolVal));
}

std::shared_ptr<JSONToken> JSONLexer::nullState(char ch)
{
    auto nullStr = std::string("n");
    nullStr += ch;
    
    auto count = 2;
    while (count > 0)
    {
        auto result = nextChar();
        if (!std::get<1>(result))
        {
            break;
        }
        nullStr += std::get<0>(result);
        count--;
    }
    
    if (nullStr != "null")
    {
        throwError("invalid literal null");
        return nullptr;
    }
    
    return std::make_shared<JSONToken>(TokenType::Null, std::move(nullStr));
}

