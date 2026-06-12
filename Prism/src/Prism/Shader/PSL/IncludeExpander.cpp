#include "prpch.h"
#include "IncludeExpander.h"
#include "Prism/Utilities/Utilities.h"

#include <fstream>
#include <sstream>
#include <filesystem>

namespace Prism::PSL
{

static std::string resolvePath(const std::string& includeName,
                                const std::string& root)
{
    auto p = std::filesystem::path(root) / includeName;
    std::error_code ec;
    if (std::filesystem::exists(p, ec) && !ec)
        return std::filesystem::canonical(p, ec).string();
    return {};
}

static std::string expandInternal(std::string source,
                                   const std::string& includeRoot,
                                   std::unordered_set<std::string>& visiting)
{
    std::string result;
    std::istringstream in(source);
    std::string line;

    while (std::getline(in, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        size_t ns = line.find_first_not_of(" \t");
        if (ns == std::string::npos || line.compare(ns, 8, "#include") != 0)
        {
            result += line + '\n';
            continue;
        }

        size_t qs = line.find('"', ns + 8);
        if (qs == std::string::npos)
        {
            result += line + '\n';
            continue;
        }
        size_t qe = line.find('"', qs + 1);
        if (qe == std::string::npos)
        {
            result += line + '\n';
            continue;
        }

        std::string includeName = line.substr(qs + 1, qe - qs - 1);
        std::string resolved = resolvePath(includeName, includeRoot);

        if (resolved.empty())
        {
            PR_CORE_WARN("IncludeExpander: 找不到 \"{0}\"", includeName);
            result += line + '\n';
            continue;
        }

        if (visiting.count(resolved))
        {
            PR_CORE_WARN("IncludeExpander: 循环 include \"{0}\"", includeName);
            result += line + '\n';
            continue;
        }

        std::string content = Prism::File::ReadFile(resolved);
        if (content.empty())
        {
            PR_CORE_WARN("IncludeExpander: 读取失败 \"{0}\"", includeName);
            result += line + '\n';
            continue;
        }

        visiting.insert(resolved);
        result += expandInternal(std::move(content), includeRoot, visiting);
        visiting.erase(resolved);
    }

    return result;
}

std::string ExpandIncludesRecursive(const std::string& filePath,
                                    const std::string& includeRoot)
{
    std::string resolved = resolvePath(filePath, includeRoot);
    if (resolved.empty())
    {
        resolved = filePath;
    }
    std::string source = Prism::File::ReadFile(resolved);
    if (source.empty())
    {
        PR_CORE_WARN("IncludeExpander: 无法读取 \"{0}\"", filePath);
        return {};
    }
    std::unordered_set<std::string> visiting;
    visiting.insert(resolved);
    return expandInternal(std::move(source), includeRoot, visiting);
}

} // namespace Prism::PSL
