#include "engine/time_utils.hpp"

#ifdef __vita__
#include <psp2/kernel/processmgr.h>
#else
#include <chrono>
#endif

namespace defender {

uint64_t now_ms() {
#ifdef __vita__
    return sceKernelGetProcessTimeWide() / 1000ULL;
#else
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
#endif
}

}

