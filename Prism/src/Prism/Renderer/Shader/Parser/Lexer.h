#pragma once
#include "Token.h"
#include <string>
#include <vector>

namespace Prism
{

struct LexerError
{
    std::string Message;
    size_t Line = 0;
    size_t Column = 0;
};

class Lexer
{
public:
    explicit Lexer(std::string source);

    std::vector<Token> Tokenize();

    const std::vector<LexerError>& GetErrors() const { return m_Errors; }
    const std::string& GetSource() const { return m_Source; }

    // Extract raw content between matching braces (handles strings and comments).
    // openBracePos must point to the '{' character in the source.
    // Returns the content between the braces (exclusive), or empty string on failure.
    static bool ExtractBlockContent(const std::string& source, size_t openBracePos, std::string& outContent);

private:
    Token NextToken();
    void SkipWhitespace();
    bool SkipComment();
    Token LexStringLiteral();
    Token LexNumber();
    Token LexIdentifierOrKeyword();

    static TokenType KeywordType(const std::string& value);

    std::string m_Source;
    size_t m_Position = 0;
    size_t m_Line = 1;
    size_t m_Column = 1;
    std::vector<LexerError> m_Errors;
};

} // namespace Prism
