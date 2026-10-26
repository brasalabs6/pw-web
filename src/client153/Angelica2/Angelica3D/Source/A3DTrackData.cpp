// Explicitly compile every original track operation for supported key types.
#include "A3DTrackData.h"
template class A3DTrackData<A3DVECTOR3>;
template class A3DTrackData<A3DQUATERNION>;
template class A3DTrackData<float>;

static_assert(sizeof(A3DTRACKSAVEDATA) == 12);
static_assert(sizeof(A3DVector3Track::SEGMENT) == 16);
static_assert(sizeof(A3DVECTOR3) == 12);
static_assert(sizeof(A3DQUATERNION) == 16);
