#pragma once

namespace zeta::core::random {

template <typename Engine>
struct EngineTraits;  // IWYU pragma: export

template <typename Engine>
constexpr bool IsConst();

template <typename Engine>
constexpr size_t GetBitCnt();

template <typename Engine>
auto Fetch(Engine& engine);

template <typename Engine>
void CheckContract(Engine& engine);

template <typename Integral, typename Engine>
Integral GetRandomInt(Engine& engine);

template <typename Engine, typename Integral>
Integral GetRandomInt(Engine& engine, Integral min, Integral max);

template <typename Engine, typename Integral>
Integral GetUnbiasedRandomInt(Engine& engine, Integral min, Integral max);

}  // namespace zeta::core::random
