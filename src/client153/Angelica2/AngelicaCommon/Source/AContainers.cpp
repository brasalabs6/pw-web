// Explicit instantiations validate complete original container interfaces.
#include "AArray.h"
#include "AList2.h"
template class AArray<int, int>;
template class AArray<void *, void *>;
template class APtrArray<int *>;
template class AList2<void *, void *>;
template class APtrList<int *>;
