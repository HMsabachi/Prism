#pragma once

#include <string>
#include "Prism/Core/Core.h"

namespace Prism {

    class PRISM_API FileSystem
    {
    public:
        /** Convert absolute path to relative (relative to working directory) */
        static std::string GetRelativePath(const std::string& absolutePath);

        /** Convert any path to absolute (resolves relative paths against working directory) */
        static std::string GetAbsolutePath(const std::string& path);
    };

}
