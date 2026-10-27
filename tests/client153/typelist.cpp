#include "ABaseDef.h"
#include "TypeList.h"
#include <type_traits>

static_assert(IsSameType<int, int>::value, "same types");
static_assert(!IsSameType<int, float>::value, "different types");
using Input = TL::TypeListGenerator2<int, float>::Result;
static_assert(std::is_same<TL::TypeAt<Input, 1>::Result, float>::value, "type at index");
using Output = TL::TypeList2ConstPtrTypeList<Input>::Result;
static_assert(std::is_same<Output::Head, const int *>::value, "first const pointer");
static_assert(std::is_same<Output::Tail::Head, const float *>::value, "second const pointer");
int main() {}
