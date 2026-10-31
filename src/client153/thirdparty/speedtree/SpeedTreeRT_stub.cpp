/*
 * SpeedTreeRT browser-build boundary (explicit failing seam).
 *
 * SpeedTreeRT is IDV's proprietary middleware. The pinned upstream tree
 * (e3d6d0075d0a745c9859f2cb25b01d71dec045) ships only the SDK headers
 * (Angelica2/Output/3rdsdk/include/SpeedTreeRT.h) and the prebuilt Windows
 * library — there is no SpeedTree source to compile, so the browser build
 * cannot contain the real implementation.
 *
 * This translation unit defines exactly the CSpeedTreeRT surface the
 * original client references (the 40 symbols of the full-closure link
 * worklist) with explicit failure semantics: loaders and factories fail,
 * getters return empty values, and the limitation is logged once. The
 * Angelica wrappers (EL_Forest.cpp, EL_Tree.cpp) take their existing error
 * paths when LoadTree fails, so the world loads without SpeedTree-rendered
 * forests.
 *
 * Declared limitation (never silent): the browser build renders no
 * SpeedTree-based forest or tree geometry until a WebGL replacement for
 * SpeedTreeRT exists. Class members the client does not call are left
 * undefined on purpose.
 *
 * This file is new code by the browser port (not an upstream adaptation);
 * it compiles against the unmodified vendored SDK header.
 */

#include <SpeedTreeRT.h>

#include <cstdio>
#include <cstring>

namespace
{

bool speedtree_limitation_logged()
{
	static bool logged = false;
	if (!logged)
	{
		logged = true;
		std::fprintf(stderr,
			"pw-browser: SpeedTreeRT is not available in the browser build "
			"(proprietary middleware, no source in the pinned tree); "
			"forest rendering is disabled.\n");
	}
	return false;
}

}  // namespace

//  Class statics (declared in the SDK header, owned by the library).

unsigned int CSpeedTreeRT::m_uiAllRefCount = 0;
bool CSpeedTreeRT::m_bTextureFlip = false;
bool CSpeedTreeRT::m_bDropToBillboard = false;
float CSpeedTreeRT::m_fHorizontalFadeStartAngle = 0.0f;
float CSpeedTreeRT::m_fHorizontalFadeEndAngle = 0.0f;
float CSpeedTreeRT::m_fHorizontalFadeValue = 0.0f;

//  Construction / destruction. The class has no virtual functions and only
//  scalar and pointer members, so zero-initialization is the inert state.

CSpeedTreeRT::CSpeedTreeRT()
{
	speedtree_limitation_logged();
	//	No vtable and only scalar/pointer members: zeroing is the inert state.
	std::memset(static_cast<void*>(this), 0, sizeof(*this));
}

CSpeedTreeRT::~CSpeedTreeRT()
{
}

void* CSpeedTreeRT::operator new(size_t nSize)
{
	return ::operator new(nSize);
}

void CSpeedTreeRT::operator delete(void* pRawMemory)
{
	::operator delete(pRawMemory);
}

//  Licensing: the key is meaningless without the library.

void CSpeedTreeRT::Authorize(const char* pKey)
{
	(void)pKey;
	speedtree_limitation_logged();
}

//  Specifying a tree model: every path that would produce geometry fails.

bool CSpeedTreeRT::Compute(const float* pTransform, unsigned int nSeed, bool bCompositeStrips)
{
	(void)pTransform;
	(void)nSeed;
	(void)bCompositeStrips;
	return false;
}

CSpeedTreeRT* CSpeedTreeRT::MakeInstance(void)
{
	return NULL;
}

bool CSpeedTreeRT::LoadTree(const char* pFilename)
{
	(void)pFilename;
	return false;
}

bool CSpeedTreeRT::LoadTree(const unsigned char* pBlock, unsigned int nNumBytes)
{
	(void)pBlock;
	(void)nNumBytes;
	return false;
}

void CSpeedTreeRT::GetTreeSize(float& fSize, float& fVariance) const
{
	fSize = 0.0f;
	fVariance = 0.0f;
}

void CSpeedTreeRT::SetTreeSize(float fNewSize, float fNewVariance)
{
	(void)fNewSize;
	(void)fNewVariance;
}

void CSpeedTreeRT::SetTreePosition(float x, float y, float z)
{
	(void)x;
	(void)y;
	(void)z;
}

//  Lighting, camera, wind, LOD: configuration of an absent engine. Every
//  entry reports the limitation at least once (the log-once helper keeps
//  the output to a single line); the configuration carries no observable
//  result because the engine it targets does not exist in this build.

void CSpeedTreeRT::SetBranchLightingMethod(ELightingMethod eMethod) { (void)eMethod; speedtree_limitation_logged(); }
void CSpeedTreeRT::SetLeafLightingMethod(ELightingMethod eMethod) { (void)eMethod; speedtree_limitation_logged(); }
void CSpeedTreeRT::SetFrondLightingMethod(ELightingMethod eMethod) { (void)eMethod; speedtree_limitation_logged(); }

void CSpeedTreeRT::SetLightState(unsigned int nLightIndex, bool bLightOn)
{
	(void)nLightIndex;
	(void)bLightOn;
	speedtree_limitation_logged();
}

void CSpeedTreeRT::SetLightAttributes(unsigned int nLightIndex, const float* pLightAttributes)
{
	(void)nLightIndex;
	(void)pLightAttributes;
	speedtree_limitation_logged();
}

void CSpeedTreeRT::SetCamera(const float* pPosition, const float* pDirection)
{
	(void)pPosition;
	(void)pDirection;
	speedtree_limitation_logged();
}

void CSpeedTreeRT::SetTime(float fTime) { (void)fTime; speedtree_limitation_logged(); }

void CSpeedTreeRT::SetLeafRockingState(bool bFlag) { (void)bFlag; speedtree_limitation_logged(); }
void CSpeedTreeRT::SetNumLeafRockingGroups(unsigned int nRockingGroups) { (void)nRockingGroups; speedtree_limitation_logged(); }
void CSpeedTreeRT::SetLeafWindMethod(EWindMethod eMethod) { (void)eMethod; speedtree_limitation_logged(); }
void CSpeedTreeRT::SetBranchWindMethod(EWindMethod eMethod) { (void)eMethod; speedtree_limitation_logged(); }
void CSpeedTreeRT::SetFrondWindMethod(EWindMethod eMethod) { (void)eMethod; speedtree_limitation_logged(); }

float CSpeedTreeRT::SetWindStrength(float fNewStrength, float fOldStrength, float fFrequencyTimeOffset)
{
	(void)fOldStrength;
	(void)fFrequencyTimeOffset;
	speedtree_limitation_logged();
	return 0.0f;
}

void CSpeedTreeRT::ComputeLodLevel(void) { speedtree_limitation_logged(); }
void CSpeedTreeRT::SetLodLevel(float fLodLevel) { (void)fLodLevel; speedtree_limitation_logged(); }
void CSpeedTreeRT::SetLodLimits(float fNear, float fFar) { (void)fNear; (void)fFar; speedtree_limitation_logged(); }
void CSpeedTreeRT::SetDropToBillboard(bool bFlag) { (void)bFlag; speedtree_limitation_logged(); }
void CSpeedTreeRT::SetTextureFlip(bool bFlag) { (void)bFlag; speedtree_limitation_logged(); }

//  LOD queries: no geometry exists, so every level count is zero.

unsigned short CSpeedTreeRT::GetNumBranchLodLevels(void) const { return 0; }
unsigned short CSpeedTreeRT::GetNumFrondLodLevels(void) const { return 0; }
unsigned short CSpeedTreeRT::GetNumLeafLodLevels(void) const { return 0; }

unsigned short CSpeedTreeRT::GetDiscreteLeafLodLevel(float fLodLevel) const
{
	(void)fLodLevel;
	return 0;
}

//  Geometry and textures: the caller only reaches these after LoadTree
//  succeeded, which never happens; the out-parameters keep their inert
//  zero-initialized state and the limitation is reported.

void CSpeedTreeRT::GetBoundingBox(float* pBounds) const
{
	if (pBounds)
		std::memset(pBounds, 0, 6 * sizeof(float));
	speedtree_limitation_logged();
}

void CSpeedTreeRT::GetGeometry(SGeometry& sGeometry, unsigned long ulBitVector,
	short sOverrideBranchLodValue, short sOverrideFrondLodValue, short sOverrideLeafLodValue)
{
	(void)sGeometry;
	(void)ulBitVector;
	(void)sOverrideBranchLodValue;
	(void)sOverrideFrondLodValue;
	(void)sOverrideLeafLodValue;
	speedtree_limitation_logged();
}

void CSpeedTreeRT::GetTextures(STextures& sTextures) const
{
	(void)sTextures;
	speedtree_limitation_logged();
}

//  Nested geometry/texture records: plain zero-initializable data holders.

CSpeedTreeRT::SGeometry::SIndexed::SIndexed()
{
	std::memset(static_cast<void*>(this), 0, sizeof(*this));
}

CSpeedTreeRT::SGeometry::SIndexed::~SIndexed()
{
}

CSpeedTreeRT::SGeometry::SLeaf::SLeaf()
{
	std::memset(static_cast<void*>(this), 0, sizeof(*this));
}

CSpeedTreeRT::SGeometry::SLeaf::~SLeaf()
{
}

CSpeedTreeRT::SGeometry::SBillboard::SBillboard()
{
	std::memset(static_cast<void*>(this), 0, sizeof(*this));
}

CSpeedTreeRT::SGeometry::SBillboard::~SBillboard()
{
}

CSpeedTreeRT::SGeometry::SGeometry()
{
	std::memset(static_cast<void*>(this), 0, sizeof(*this));
}

CSpeedTreeRT::SGeometry::~SGeometry()
{
}

CSpeedTreeRT::STextures::STextures()
{
	std::memset(static_cast<void*>(this), 0, sizeof(*this));
}

CSpeedTreeRT::STextures::~STextures()
{
}
