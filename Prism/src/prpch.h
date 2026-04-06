#pragma once
// Precompiled header files contain the necessary system header files.
// 预编译头文件 包含必要的系统头文件

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <limits>
#include <sstream>
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Prism/Core/Log.h"
#include "Prism/Core/Core.h"
#include "Prism/Debug/Instrumentor.h"

#ifdef PR_PLATFORM_WINDOWS
	#include <Windows.h>
#endif

