#include "A3DObject.h"
#include "AArray.h"
#include "AList2.h"
#include "ALog.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <type_traits>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x))                                                                  \
      throw std::runtime_error(#x);                                            \
  } while (false)
std::string captured;
void sink(const char *text) { captured = text; }
int main() {
  static_assert(!std::is_copy_constructible_v<AArray<int, int>>);
  static_assert(!std::is_copy_constructible_v<APtrList<int *>>);
  A3DObject object;
  object.SetName("original bone");
  CHECK(std::string(object.GetName()) == "original bone");
  CHECK(object.GetClassID() == A3D_CID_UNKNOWN);
  auto previous = a_RedirectDefLogOutput(sink);
  a_LogOutput(1, "original %d", 153);
  CHECK(captured == "<!> original 153\n");
  a_LogOutputNoReturn(0, "same %s", "client");
  CHECK(captured == "<-> same client");
  a_SendLogToDebugWnd(0, "debug %d", 42);
  CHECK(captured.find("debug 42") != std::string::npos);
  a_LogOutput(0, "counter=%I64d", 1234567890123LL);
  CHECK(captured == "<-> counter=1234567890123\n");
  std::string oversized(6000, 'x');
  a_LogOutput(0, "%s", oversized.c_str());
  CHECK(captured.size() <= 2052);
  CHECK(captured.back() == '\n');
  a_RedirectDefLogOutput(previous);
  ALog::SetLogDir("client153-test-logs");
  ALog log;
  CHECK(!log.Init("missing/invalid.log", "test"));
  CHECK(!log.Init("missing\\invalid.log", "test"));
  CHECK(log.Init("engine.log", "Original Common"));
  CHECK(log.Log("value=%d", 42));
  CHECK(log.Release());
  std::ifstream file("client153-test-logs/engine.log");
  std::string content((std::istreambuf_iterator<char>(file)), {});
  CHECK(content.find("value=42") != std::string::npos);
  AArray<int, int> array;
  CHECK(array.Add(7) == 0);
  CHECK(array.Add(9) == 1);
  CHECK(array.GetSize() == 2);
  CHECK(array[0] == 7);
  array.RemoveAt(0);
  CHECK(array[0] == 9);
  int first = 1, second = 2;
  APtrArray<int *> pointers, extra;
  pointers.Add(&first);
  extra.Add(&second);
  CHECK(pointers.Append(extra) == 1);
  CHECK(*pointers.ElementAt(1) == &second);
  APtrList<int *> list;
  auto a = list.AddTail(&first);
  list.AddTail(&second);
  CHECK(list.GetCount() == 2);
  CHECK(list.GetHead() == &first);
  list.RemoveAt(a);
  CHECK(list.GetHead() == &second);
  APtrList<int *> tail;
  tail.AddTail(&first);
  list.AddListToTail(&tail);
  CHECK(list.GetCount() == 2);
  CHECK(list.GetTail() == &first);
  list.RemoveAll();
  CHECK(list.GetCount() == 0);
  std::remove("client153-test-logs/engine.log");
  std::cout << "PASS original Common log delivery, file errors, AArray and "
               "pooled pointer-list operations\n";
}
