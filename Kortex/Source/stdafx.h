#pragma once
#pragma comment(lib, "delayimp")

/* KxFramework */
#include <KxFramework/KxFramework.h>
#include <KxFramework/KxWinUndef.h>

#if _WIN64
#pragma comment(lib, "Bin/KxFramework x64.lib")
#else
#pragma comment(lib, "Bin/KxFramework.lib")
#endif

// C++
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <stack>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <functional>
#include <variant>
#include <optional>
#include <locale>
#include <limits>
#include <type_traits>

/* Kortex */
#define KIPC_SERVER 0
#include <Kortex/Core.hpp>

enum KLayoutConstants
{
	KLC_VERTICAL_SPACING = 3,
	KLC_HORIZONTAL_SPACING = 3,

	KLC_VERTICAL_SPACING_SMALL = 2,
	KLC_HORIZONTAL_SPACING_SMALL = 2
};
