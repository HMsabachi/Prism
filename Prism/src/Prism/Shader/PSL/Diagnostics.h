#pragma once

#include "Prism/Shader/ShaderCommon.h"

#include <string>
#include <vector>

namespace Prism::PSL
{

enum class Severity
{
    Warning,
    Error,
    Fatal,
};

struct Diagnostic
{
    Severity Level = Severity::Error;
    std::string Message;
    SourceLocation Loc;
};

class DiagnosticCollector
{
public:
    void Warning(const std::string& msg, const SourceLocation& loc = {});
    void Error(const std::string& msg, const SourceLocation& loc = {});
    void Fatal(const std::string& msg, const SourceLocation& loc = {});

    bool HasErrors() const;
    bool HasWarnings() const;

    const std::vector<Diagnostic>& GetDiagnostics() const { return m_Diagnostics; }
    void Clear();

    void PrintAll() const;

private:
    std::vector<Diagnostic> m_Diagnostics;
    bool m_HasFatal = false;
};

} // namespace Prism::PSL
