#pragma once
#include "seq_common.h"
#include "seq_ease.h"
#include <algorithm>

namespace seq
{

//---------------------------------------------------------------------------------------
/// Return a value between 'start' and 'end', based on progress(0.0f - 1.0f)
//---------------------------------------------------------------------------------------
template<typename T>
auto lerp(const T& start, const T& end, float progress, const ease_t& ease_func = ease::linear) -> T;

//---------------------------------------------------------------------------------------
/// Return a value between 'out_start' and 'out_end',
/// based on percent of 'in' between 'in_start' and 'in_end'.
//---------------------------------------------------------------------------------------
template<typename InType, typename OutType>
auto range_map(const InType& in,
               const decltype(in)& in_start,
               const decltype(in)& in_end,
               const OutType& out_start,
               const decltype(out_start)& out_end,
               const ease_t& ease_func = ease::linear) -> OutType;

template<typename T>
auto clamp(const T& value, const T& min, const T& max) -> T
{
    return std::min<T>(std::max<T>(value, min), max);
}

// Make things constexpr
auto square(float x, int n) -> float;
auto flip(float x) -> float;
auto mix(float a, float b, float weight, float t) -> float;
auto crossfade(float a, float b, float t) -> float;
auto scale(float a, float t) -> float;
auto reverse_scale(float a, float t) -> float;
auto arch(float t) -> float;

} // namespace seq

#include "seq_math.hpp"
