#pragma once

// Lexer 已重构为三层架构:
//   SourceManager  → 偏移量定位 + 换行表
//   TokenStream    → 惰性词法分析 + 关键字查找
//   Parser         → 递归下降语法分析
//
// 旧 Lexer 的 Tokenize() / ExtractBlock() / InitKeywordMap() 已删除。

#include "SourceManager.h"
#include "TokenStream.h"

namespace Prism::PSL
{

// 便捷工厂：创建完整的词法分析流水线
struct Lexer
{
    DiagnosticCollector Diag;
    SourceManager SM;
    TokenStream   Stream;

    Lexer() : SM(), Stream(SM, &Diag) {}

    // 从文件加载
    bool Load(const std::string& filePath) {
        SM = SourceManager(filePath);
        if (!SM.IsValid()) return false;
        Stream = TokenStream(SM, &Diag);
        return true;
    }
};

} // namespace Prism::PSL
