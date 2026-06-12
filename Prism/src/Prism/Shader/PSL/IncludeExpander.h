#pragma once

#include <string>

namespace Prism::PSL
{

std::string ExpandIncludesRecursive(const std::string& filePath,
                                    const std::string& includeRoot);

} // namespace Prism::PSL
