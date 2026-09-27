#pragma once

#include <zeta/core/debug_utils/logging.hpp>

namespace zeta::core::debug_utils::diag {

inline std::ostream& diag_os{ std::cout };
extern logging::CircularLogger diag_logger;

constexpr size_t assert_block_banner_width{ 196 };

#define ZETA_Core_DebugUtils_Diag_BannerColorCode "\033[38;5;196m"

}  // namespace zeta::core::debug_utils::diag
