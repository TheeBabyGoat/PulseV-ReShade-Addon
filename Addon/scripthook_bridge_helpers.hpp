#pragma once
#include "scripthook_bridge.hpp"
#include <utility>

namespace pv::scripthook_helpers {

// Run a callable on the ScriptHookV script thread and return its result synchronously.
template<typename F>
auto shv_sync(F&& f) -> decltype(f()) {
    using R = decltype(f());
    return pv::clouds::EnqueueScriptTaskSync<R>(std::forward<F>(f));
}

// Run a callable on the ScriptHookV script thread asynchronously (fire-and-forget).
inline void shv_async(std::function<void()> f) {
    pv::clouds::EnqueueScriptTask(std::move(f));
}

} // namespace pv::scripthook_helpers
