#include "Windows.h"
#include <emscripten/heap.h>
#include <cstdint>
#include <cstdlib>

int main() {
  auto *buffer = static_cast<BYTE *>(std::malloc(32));
  if (!buffer) return 1;
  if (IsBadWritePtr(buffer, 32) || IsBadReadPtr(buffer + 8, 8)) return 2;
  const auto end = emscripten_get_heap_size();
  if (!IsBadWritePtr(nullptr, 1) || !IsBadReadPtr(reinterpret_cast<void *>(end - 1), 2) ||
      !IsBadWritePtr(reinterpret_cast<void *>(UINTPTR_MAX - 2), 8)) return 3;
  std::free(buffer);
  return 0;
}
