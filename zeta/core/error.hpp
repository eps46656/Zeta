#pragma once

namespace zeta::core::error {

struct Error;

using Analyzer = void (*)(Error const& error);

constexpr unsigned char empty_message[1]{ 0 };

struct Error {
    unsigned char const* message;

    Analyzer analyzer;

    unsigned char payload[256];
};

}  // namespace zeta::core::error
