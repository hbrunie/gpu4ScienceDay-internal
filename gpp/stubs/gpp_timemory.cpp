
#include <cstdint>

extern "C" void
gpp_init_library(const int, const uint64_t, const uint32_t, void*)
{
}

extern "C" void
gpp_finalize_library()
{
}

extern "C" void
gpp_begin_record(const char*, uint32_t, uint64_t*)
{
}

extern "C" void gpp_end_record(uint64_t) {}
