#include "JSONToken.h"

JSONToken::JSONToken(TokenType _type, std::string _content)
{
    type = _type;
    content = _content;
}

bool JSONToken::isElementType()
{
    return type == TokenType::Null || type == TokenType::String || isNumberType();
}

bool JSONToken::isContainer()
{
    return type == TokenType::LeftBracket || type == TokenType::LeftBrace;
}

bool JSONToken::isNumberType()
{
    return type == TokenType::Integer || type == TokenType::Float || type == TokenType::Boolean;
}

bool JSONToken::isValueType()
{
    return isContainer() || isElementType();
}
