#include "Windows.h"
#include <type_traits>

static_assert(std::is_pointer<LPEXCEPTION_POINTERS>::value, "exception information is an opaque pointer");
static_assert(std::is_same<LPEXCEPTION_POINTERS, _EXCEPTION_POINTERS *>::value, "exception pointer tag");
static_assert(std::is_same<PVOID, void *>::value, "Win32 PVOID type");
int main() {}
