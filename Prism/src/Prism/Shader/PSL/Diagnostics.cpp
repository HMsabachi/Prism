#include "prpch.h"
#include "Diagnostics.h"

namespace Prism::PSL
{

void DiagnosticCollector::Warning(const std::string& msg, const SourceLocation& loc)
{
    m_Diagnostics.push_back({Severity::Warning, msg, loc});
}

void DiagnosticCollector::Error(const std::string& msg, const SourceLocation& loc)
{
    m_Diagnostics.push_back({Severity::Error, msg, loc});
}

void DiagnosticCollector::Fatal(const std::string& msg, const SourceLocation& loc)
{
    m_HasFatal = true;
    m_Diagnostics.push_back({Severity::Fatal, msg, loc});
}

bool DiagnosticCollector::HasErrors() const
{
    for (auto& d : m_Diagnostics)
        if (d.Level == Severity::Error || d.Level == Severity::Fatal)
            return true;
    return false;
}

bool DiagnosticCollector::HasWarnings() const
{
    for (auto& d : m_Diagnostics)
        if (d.Level == Severity::Warning)
            return true;
    return false;
}

void DiagnosticCollector::Clear()
{
    m_Diagnostics.clear();
    m_HasFatal = false;
}

void DiagnosticCollector::PrintAll() const
{
    for (auto& d : m_Diagnostics)
    {
        bool isError = (d.Level == Severity::Error || d.Level == Severity::Fatal);
        if (!d.Loc.FilePath.empty())
        {
            if (isError)
                PR_CORE_ERROR("[Shader Error] {}:{}:{}: {}", d.Loc.FilePath, d.Loc.Line, d.Loc.Column, d.Message);
            else
                PR_CORE_WARN("[Shader Warning] {}:{}:{}: {}", d.Loc.FilePath, d.Loc.Line, d.Loc.Column, d.Message);
        }
        else if (d.Loc.Line > 0)
        {
            if (isError)
                PR_CORE_ERROR("[Shader Error] Line {}:{}: {}", d.Loc.Line, d.Loc.Column, d.Message);
            else
                PR_CORE_WARN("[Shader Warning] Line {}:{}: {}", d.Loc.Line, d.Loc.Column, d.Message);
        }
        else
        {
            if (isError)
                PR_CORE_ERROR("[Shader Error] {}", d.Message);
            else
                PR_CORE_WARN("[Shader Warning] {}", d.Message);
        }
    }
}

} // namespace Prism::PSL
