#ifndef STRINGS_H
#define STRINGS_H

#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <cstring>

class Strings
{
  public:
    template <typename CharType>
    static std::vector<std::basic_string<CharType>> split(const std::basic_string<CharType> src, const std::basic_string<CharType> token)
    {
        using StrType = std::basic_string<CharType>;
        auto srcSize = src.size();
        auto tokenSize = token.size();
        auto result = std::vector<StrType>();

        if (srcSize == 0 || tokenSize == 0)
        {
            return result;
        }

        for (auto i = 0; i < srcSize; ++i)
        {
            auto tokenIndex = src.find(token, i);
            if (tokenIndex == StrType::npos)
            {
                if (i < srcSize)
                {
                    auto subStr = src.substr(i);
                    result.push_back(std::move(subStr));
                }
                break;
            }
            else if (tokenIndex < srcSize)
            {
                auto subStr = src.substr(i, tokenIndex - i);
                result.push_back(std::move(subStr));

                i = tokenIndex - 1 + tokenSize;
            }
        }

        return result;
    }

    template <typename CharType, typename IterType>
    static std::basic_string<CharType> join(IterType begin, IterType end, const std::basic_string<CharType> token)
    {
        if (begin == end)
        {
            return std::basic_string<CharType>();
        }

        auto result = std::basic_string<CharType>();
        auto pos = end - 1;
        while (begin != pos)
        {
            result += *begin + token;
            ++begin;
        }

        return result + *pos;
    }

    template <typename StrType>
    static StrType join(const std::vector<StrType> srcArr, const StrType token)
    {
        return join(std::begin(srcArr), std::end(srcArr), token);
    }

    template <typename CharType>
    static std::basic_string<CharType> toLowerCase(const std::basic_string<CharType> str)
    {
        auto result = std::basic_string<CharType>(str.size(), ' ');
        std::transform(std::begin(str), std::end(str), std::begin(result), tolower);
        return result;
    }

    template <typename CharType>
    static std::basic_string<CharType> toUpperCase(const std::basic_string<CharType> str)
    {
        auto result = std::basic_string<CharType>(str.size(), ' ');
        std::transform(std::begin(str), std::end(str), std::begin(result), toupper);
        return result;
    }

    template <typename StrType>
    static bool isPrefix(const StrType str, const StrType prefix)
    {
        auto strSize = str.size();
        auto prefixSize = prefix.size();
        if (strSize == 0 || strSize < prefixSize)
        {
            return false;
        }
        else if (prefixSize == 0)
        {
            return true;
        }

        return strncmp(str.c_str(), prefix.c_str(), prefixSize) == 0;
    }

    template <typename StrType>
    static bool isSuffix(const StrType str, const StrType suffix)
    {
        auto strSize = str.size();
        auto suffixSize = suffix.size();
        if (strSize == 0 || strSize < suffixSize)
        {
            return false;
        }
        else if (suffixSize == 0)
        {
            return true;
        }

        auto substr = &(str.c_str()[strSize - suffixSize]);
        return strncmp(substr, suffix.c_str(), suffixSize) == 0;
    }

    template <typename CharType>
    static std::basic_string<CharType> trimLeft(const std::basic_string<CharType> src, const CharType ch = CharType(' '))
    {
        auto iter = std::begin(src);
        auto end = std::end(src);
        while (iter != end && *iter == ch)
        {
            ++iter;
        }
        return std::basic_string<CharType>(iter, end);
    }

    template <typename CharType>
    static std::basic_string<CharType> trimRight(const std::basic_string<CharType> src, const CharType ch = CharType(' '))
    {
        auto riter = std::rbegin(src);
        auto rend = std::rend(src);
        while (riter != rend && *riter == ch)
        {
            ++riter;
        }
        return std::basic_string<CharType>(std::begin(src), riter.base());
    }
};

#endif
