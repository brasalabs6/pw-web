#include "AArray.h"
#include <stdexcept>

struct Item { int value; };
int main() {
  AArray<Item, Item &> source;
  AArray<Item, Item &> target;
  Item first{7}, second{2};
  source.Add(first);
  target.Add(second);
  const AArray<Item, Item &> &read_only = source;
  target.SetAt(0, read_only[0]);
  if (target[0].value != 7) throw std::runtime_error("const array element copy");
}
