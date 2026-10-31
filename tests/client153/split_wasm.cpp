#include "CSplit.h"

int main() {
  CSplit input(L"100,36672,1");
  auto values = input.Split(L",");
  if (values.size() != 3 || values[0].ToInt() != 100 ||
      values[1].ToInt() != 36672 || values[2].ToInt() != 1) return 1;
  CSplit range(L"1-10");
  auto limits = range.Split(L"-");
  return limits.size() == 2 && limits[0].ToInt() == 1 && limits[1].ToInt() == 10 ? 0 : 2;
}
