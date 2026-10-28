//	The COM activation registry for the ATL seam. The browser-dialog
//	adapter registers the web view class in the UI cycle; until then,
//	activation fails explicitly (no fake success).
#include "atlbase.h"

namespace pw153::platform {
struct ComRegistryEntry {
  const void *classId;
  HRESULT (*create)(void **result);
};
} // namespace pw153::platform

//	The registry (empty in this cycle; the UI adapter appends entries).
static const pw153::platform::ComRegistryEntry pw153_com_registry[1] = {
    {nullptr, nullptr}};

HRESULT __CoCreateInstance(const void *classId, void **result) {
  if (result)
    *result = nullptr;
  if (!classId)
    return E_FAIL;
  for (const auto &entry : pw153_com_registry) {
    if (entry.classId == classId && entry.create)
      return entry.create(result);
  }
  //	No registered adapter for this class: explicit failure.
  return E_FAIL;
}

HRESULT CoInitialize(void *reserved) {
  (void)reserved;
  //	The port's COM runtime initializes lazily; startup is a no-op success
  //	with the real backend semantics.
  return S_OK;
}

void CoUninitialize() {}

//	VariantClear: the real reset (frees a BSTR value, empties the rest).
HRESULT VariantClear(VARIANT *variant) {
  if (!variant)
    return E_INVALIDARG;
  if (variant->vt == VT_BSTR && variant->bstrVal)
    SysFreeString(variant->bstrVal);
  variant->vt = VT_EMPTY;
  variant->bstrVal = nullptr;
  return S_OK;
}
