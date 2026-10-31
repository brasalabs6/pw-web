// Structured asset-failure taxonomy (the compatibility-runtime review's
// M1 diagnostic): unique failing paths counted by reason category, dumped
// once at boot end instead of one stack trace per call site.
#ifndef PW153_PLATFORM_ASSET_DIAG_H
#define PW153_PLATFORM_ASSET_DIAG_H

namespace pw153::platform {

enum class AssetFailReason {
  NoPackageCoverage = 0,	// no open package folder claims the path
  PckEntryNotFound = 1,		// a package claims the folder but the index missed
  DiskOpenFailed = 2,		// the loose-file fopen failed
  DiskReadFailed = 3,		// the loose file opened but the read failed
};

void RecordAssetFailure(AssetFailReason reason, const char *path);
void DumpAssetFailures();

}  // namespace pw153::platform

extern "C" void pw153_asset_diag_dump();

#endif	// PW153_PLATFORM_ASSET_DIAG_H
