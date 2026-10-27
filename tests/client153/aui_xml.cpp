#include "AUI.h"
#include "AXMLFile.h"
#include "widestr.h"
int main() {
  AXMLItem item;
  item.SetName(L"Label");
  item.SetValue(L"Name", L"Example");
  if (pw153::platform::wide_strcmp(item.GetItemText(), L"<Label Name=\"Example\">") != 0)
    return 1;
  AXMLItem parsed;
  if (!parsed.Init(L"<Label Name=\"Example\">")) return 2;
  const ACHAR *value = parsed.GetValue(L"Name");
  return !value || pw153::platform::wide_strcmp(value, L"Example") != 0;
}
