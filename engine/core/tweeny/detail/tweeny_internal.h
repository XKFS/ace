#pragma once

namespace tweeny
{
struct tween_manager;

namespace detail
{
auto get_manager() -> tween_manager&;

void push(tween_manager& mgr);
void pop();

} // namespace detail
} // namespace tweeny
