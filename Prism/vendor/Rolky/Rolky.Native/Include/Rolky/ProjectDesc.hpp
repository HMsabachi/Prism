#pragma once

#include "Core.hpp"
#include "String.hpp"

namespace Rolky {

    struct ProjectDesc
    {
        String Name = {};
        String Directory = {};
        String OutputDirectory = {};
        String SourceFiles = {};
        String ReferencePaths = {};
        String Defines = {};
        Bool32 AllowUnsafe = true;
    };

}
