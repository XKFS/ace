#include "tweeny_internal.h"

#include "../tween_manager.h"
#include <vector>

namespace tweeny
{
namespace detail
{

auto get_global_manager() -> tween_manager&
{
    static tween_manager manager_;
    return manager_;
}

auto get_manager_stack() -> std::vector<tween_manager*>&
{
    static std::vector<tween_manager*> stack{&get_global_manager()};
    return stack;
}

auto get_manager() -> tween_manager&
{
    auto& stack = get_manager_stack();
    return *stack.back();

}

void push(tween_manager& mgr)
{
    auto& stack = get_manager_stack();
    stack.push_back(&mgr);
}
void pop()
{
    auto& stack = get_manager_stack();
    if(stack.size() > 1)
    {
        stack.pop_back();
    }
}

} // namespace detail
} // namespace tweeny
