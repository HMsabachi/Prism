#pragma once
// Precompiled header files contain the necessary system header files.
// 预编译头文件 包含必要的系统头文件
#pragma warning(disable: 4251)
#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <limits>
#include <sstream>
#include <vector>
#include <queue>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Prism/Core/Log.h"
#include "Prism/Core/Core.h"
#include "Prism/Core/Time.h"
#include "Prism/Debug/Instrumentor.h"
#include "Prism/Core/Buffer.h"


#include "Prism/Utilities/Utilities.h"
#include "Prism/Utilities/BitFlags.h"
#ifdef PR_PLATFORM_WINDOWS
	#include <Windows.h>
#endif

