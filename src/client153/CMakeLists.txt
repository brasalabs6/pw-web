# Original Common interface, with portable string storage/formatting corrections.
#	Emscripten ships zlib in its sysroot (link -lz directly); native builds
#	use the system zlib through CMake's FindZLIB.
if(EMSCRIPTEN)
  #	Emscripten's zlib port flags live in the top-level CMakeLists so test
  #	targets outside this directory scope link the port too.
  set(PW153_ZLIB_LINK "")
else()
  find_package(ZLIB REQUIRED)

#	The original client is a UTF-16 (MSVC wchar_t = 16-bit) build; the whole
#	port compiles with the same width so the original wide sources are kept.
add_compile_options(-fshort-wchar)
  set(PW153_ZLIB_LINK ZLIB::ZLIB)
endif()

add_library(client153_platform STATIC platform/common/clock.cpp platform/common/format.cpp platform/common/utf16.cpp platform/common/widestr.cpp platform/common/keystate.cpp platform/common/findfiles.cpp platform/common/toolhelp.cpp platform/common/atlcom_registry.cpp platform/common/inifile.cpp platform/common/dsound.cpp)
target_include_directories(client153_platform PUBLIC platform/common)
target_include_directories(client153_platform PUBLIC Angelica2/AngelicaCommon/Header)
target_compile_features(client153_platform PUBLIC cxx_std_17)
target_compile_options(client153_platform PRIVATE -Wall -Wextra -Werror -pedantic "$<$<CXX_COMPILER_ID:GNU>:-Wno-template-body>")
add_library(client153_common STATIC
  Angelica2/AngelicaCommon/Source/AWString.cpp
  Angelica2/AngelicaCommon/Source/AString.cpp
  Angelica2/AngelicaCommon/Source/ATime.cpp
  Angelica2/AngelicaCommon/Source/ALog.cpp
  Angelica2/AngelicaCommon/Source/AContainers.cpp
  Angelica2/AngelicaCommon/Source/AAssist.cpp
  Angelica2/AngelicaCommon/Source/ACounter.cpp)
target_link_libraries(client153_common PUBLIC client153_platform)	#	PUBLIC: wide seam headers are part of the common surface.
target_include_directories(client153_common PUBLIC Angelica2/AngelicaCommon/Header)
target_compile_features(client153_common PUBLIC cxx_std_17)
#	The original debug-new pattern (#define new A_DEBUG_NEW) hides a keyword;
#	clang flags it where GCC does not.
target_compile_options(client153_common PRIVATE -Wall -Wextra -Werror -pedantic "$<$<CXX_COMPILER_ID:GNU>:-Wno-template-body>" "$<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wno-error=keyword-macro>")
if(EMSCRIPTEN)
  target_compile_options(client153_platform PUBLIC -fexceptions)
  target_link_options(client153_platform INTERFACE -fexceptions)
  target_compile_options(client153_common PUBLIC -fexceptions)
  target_link_options(client153_common INTERFACE -fexceptions)
endif()

# Original engine compilation target; linking the complete renderer is still pending.
if(EMSCRIPTEN)
  add_library(client153_engine_original OBJECT EXCLUDE_FROM_ALL
    Angelica2/Angelica3D/Source/A3DESPMove.cpp
    Angelica2/Angelica3D/Source/A3DTerrain2Loader.cpp
    Angelica2/Angelica3D/Source/A3DHermiteSpline.cpp
    Angelica2/Angelica3D/Source/A3DHorizonCull.cpp
    Angelica2/Angelica3D/Source/A3DImgModel.cpp
    Angelica2/Angelica3D/Source/A3DModelCollector.cpp
    Angelica2/AngelicaFile/Source/ATextTabFile.cpp
    Angelica2/Angelica3D/Source/A3DCamera.cpp
    Angelica2/Angelica3D/Source/A3DCameraBase.cpp
    Angelica2/Angelica3D/Source/A3DOrthoCamera.cpp
    Angelica2/Angelica3D/Source/A3DMaterial.cpp
    Angelica2/Angelica3D/Source/A3DMathUtility.cpp
    Angelica2/Angelica3D/Source/A3DLightMan.cpp
    Angelica2/Angelica3D/Source/A3DFrustum.cpp
    Angelica2/Angelica3D/Source/A3DViewport.cpp
    Angelica2/Angelica3D/Source/A3DGeometry.cpp
    Angelica2/Angelica3D/Source/A2DSprite.cpp
    Angelica2/Angelica3D/Source/A2DSpriteItem.cpp
    Angelica2/Angelica3D/Source/A2DSpriteBuffer.cpp
    Angelica2/Angelica3D/Source/A2DSpriteTexMan.cpp
    Angelica2/Angelica3D/Source/A2DSpriteTextures.cpp
    Angelica2/Angelica3D/Source/A3DSkinModel.cpp
    Angelica2/Angelica3D/Source/A3DSkin.cpp
    Angelica2/Angelica3D/Source/A3DLight.cpp
    Angelica2/Angelica3D/Source/A3DFlatCollector.cpp
    Angelica2/Angelica3D/Source/A3DTerrain2.cpp
    Angelica2/Angelica3D/Source/A3DTerrain2Env.cpp
    Angelica2/Angelica3D/Source/A3DTerrain2Trace.cpp
    Angelica2/Angelica3D/Source/A3DTerrainWater.cpp
    Angelica2/Angelica3D/Source/A3DTerrain2Render.cpp
    Angelica2/Angelica3D/Source/A3DTerrain2Blk.cpp
    Angelica2/Angelica3D/Source/A3DWaterArea.cpp
    Angelica2/Angelica3D/Source/A3DEngine.cpp
    Angelica2/Angelica3D/Source/A3DTextureMan.cpp
    Angelica2/Angelica3D/Source/A3DShader.cpp
    Angelica2/Angelica3D/Source/A3DShaderMan.cpp
    Angelica2/Angelica3D/Source/A3DControl.cpp
    Angelica2/Angelica3D/Source/A3DDevice.cpp
    Angelica2/Angelica3D/Source/A3DCursor.cpp
    Angelica2/Angelica3D/Source/A3DFontMan.cpp
    Angelica2/Angelica3D/Source/A3DWireCollector.cpp
    Angelica2/Angelica3D/Source/A3DGDI.cpp
    Angelica2/Angelica3D/Source/A3DLitModelRender.cpp
    Angelica2/Angelica3D/Source/A3DShadowMap.cpp
    Angelica2/Angelica3D/Source/A3DLitModel.cpp
    Angelica2/Angelica3D/Source/A3DStream.cpp
    Angelica2/Angelica3D/Source/A3DTexture.cpp
    Angelica2/Angelica3D/Source/A3DSkinMan.cpp
    Angelica2/Angelica3D/Source/A3DSkinModelAux.cpp
    Angelica2/Angelica3D/Source/A3DAbsTrackMaker.cpp
    Angelica2/Angelica3D/Source/A3DGFXCollector.cpp
    Angelica2/Angelica3D/Source/A3DLamp.cpp
    Angelica2/Angelica3D/Source/A3DMesh.cpp
    Angelica2/Angelica3D/Source/A3DMorphRigidMesh.cpp
    Angelica2/Angelica3D/Source/A3DMorphSkinMesh.cpp
    Angelica2/Angelica3D/Source/A3DMuscleMesh.cpp
    Angelica2/Angelica3D/Source/A3DModel.cpp
    Angelica2/Angelica3D/Source/A3DGraphicsFX.cpp
    Angelica2/Angelica3D/Source/A3DPlants.cpp
    Angelica2/Angelica3D/Source/A3DRigidMesh.cpp
    Angelica2/Angelica3D/Source/A3DSkinMesh.cpp
    Angelica2/Angelica3D/Source/A3DSky.cpp
    Angelica2/Angelica3D/Source/A3DSuppleMesh.cpp
    Angelica2/Angelica3D/Source/A3DTerrain.cpp
    Angelica2/AngelicaCommon/Source/AList.cpp
    Angelica2/AngelicaFile/Source/AWIniFile.cpp
    Angelica2/AngelicaFile/Source/AWScriptFile.cpp
    Angelica2/AngelicaMedia/Source/AMSoundBufferMan.cpp
    Angelica2/AngelicaMedia/Source/AMConfig.cpp
    Angelica2/AngelicaMedia/Source/AMPI.cpp
    Angelica2/AngelicaMedia/Source/AMEngine.cpp
    Angelica2/AngelicaMedia/Source/AMSoundEngine.cpp
    Angelica2/Angelica3D/Source/A3DTerrain2RenderAux.cpp
    Angelica2/Angelica3D/Source/A3DWaterWave.cpp
    Angelica2/Angelica3D/Source/A3DTerrainWaterRender.cpp
    Angelica2/Angelica3D/Source/A3DSurface.cpp
    Angelica2/Angelica3D/Source/A3DSurfaceMan.cpp
    Angelica2/Angelica3D/Source/A3DFont.cpp
    Angelica2/Angelica3D/Source/a3dskinrender.cpp
    Angelica2/Angelica3D/Source/A3DCollision.cpp
    Angelica2/Angelica3D/Source/A3DFrame.cpp
    Angelica2/Angelica3D/Source/A3DGFXMan.cpp
    Angelica2/Angelica3D/Source/A3DRenderTarget.cpp
    Angelica2/Angelica3D/Source/A3DVertexShader.cpp
    Angelica2/Angelica3D/Source/A3DPixelShader.cpp
    Angelica2/Angelica3D/Source/A3DSkinMeshMan.cpp
    Angelica2/Angelica3D/Source/A3DSkinModelAct.cpp
    Angelica2/Angelica3D/Source/A3DSkySphere.cpp
    Angelica2/Angelica3D/Source/A3DTrace.cpp
    Angelica2/Angelica3D/Source/A3DVectorComp.cpp
    Angelica2/Angelica3D/Source/A3DVertexCollector.cpp
    Angelica2/Angelica3D/Source/A3DWorld.cpp
    Angelica2/AngelicaCommon/Source/APerlinNoise1D.cpp
    Angelica2/AngelicaCommon/Source/ARandomGen.cpp
    Angelica2/AngelicaFile/Source/AFileUnicode.cpp
    Angelica2/AngelicaFile/Source/AMemFile.cpp
    Angelica2/AngelicaMedia/Source/AMVideoClip.cpp
    Angelica2/AngelicaMedia/Source/AMSoundStream.cpp
    Angelica2/AngelicaMedia/Source/AMVideoEngine.cpp
    Angelica2/AngelicaMedia/Source/AMSoundBuffer.cpp
    Angelica2/AngelicaMedia/Source/AMSoundDevice.cpp
    Angelica2/AngelicaMedia/Source/AM3DSoundBuffer.cpp
    Angelica2/AngelicaMedia/Source/AM3DSoundDevice.cpp
    Angelica2/AngelicaMedia/Source/AMOggFile.cpp
    Angelica2/AngelicaMedia/Source/AMMp3File.cpp
    Angelica2/AngelicaMedia/Source/AMWaveFile.cpp
    Angelica2/AngelicaMedia/Source/FFT.cpp
    Angelica2/ImmWrapper/Source/AMImmEffect.cpp
    Angelica2/ImmWrapper/Source/AMImmWrapper.cpp
    Angelica2/Angelica3D/Source/A3DBillBoard.cpp
    Angelica2/Angelica3D/Source/A3DSuperDecal.cpp
    Angelica2/Angelica3D/Source/A3DMeshCollector.cpp
    Angelica2/Angelica3D/Source/A3DDecal.cpp
    Angelica2/Angelica3D/Source/A3DLensFlare.cpp
    Angelica2/Angelica3D/Source/A3DMeshSorter.cpp
    Angelica2/Angelica3D/Source/A3DClothMesh.cpp
    Angelica2/Angelica3D/Source/A3DMorphMesh.cpp
    Angelica2/Angelica3D/Source/A3DMeshBase.cpp
    Angelica2/Angelica3D/Source/A3DModelMan.cpp
    Angelica2/Angelica3D/Source/A3DMoxMan.cpp
    Angelica2/Angelica3D/Source/A3DMuscleOrgan.cpp
    Angelica2/Angelica3D/Source/A3DTerrain2LOD.cpp
    Angelica2/Angelica3D/Source/A3DSkinModelMan.cpp
    Angelica2/Angelica3D/Source/A3DImgModelMan.cpp
    Angelica2/AngelicaCommon/Source/APerlinNoiseBase.cpp
    Angelica2/Angelica3D/Source/A3DRain.cpp
    Angelica2/Angelica3D/Source/A3DSnow.cpp
    Angelica2/Angelica3D/Source/A3DParticleSystem.cpp
    Angelica2/Angelica3D/Source/A3DStars.cpp
    Angelica2/Angelica3D/Source/A3DWorldCDS.cpp
    Angelica2/Angelica3D/Source/A3DScene.cpp
    Angelica2/Angelica3D/Source/A3DSuperSpray.cpp
    Angelica2/Angelica3D/Source/A3DCubeTexture.cpp
    Angelica2/Angelica3D/Source/A3DESP.cpp
    Angelica2/Angelica3D/Source/A3DPArray.cpp
    Angelica2/Angelica3D/Source/A3DTerrainOutline.cpp
    Angelica2/Angelica3D/Source/A3DTerrain2CullHC.cpp
    Angelica2/Angelica3D/Source/A3DIBLScene.cpp
    Angelica2/Angelica3D/Source/A3DTerrain2LoaderA.cpp
    Angelica2/Angelica3D/Source/A3DTerrain2LoaderB.cpp
    Angelica2/Angelica3D/Source/A3DTerrainWaterSimpleAlphaRender.cpp)
  target_include_directories(client153_engine_original PRIVATE
    platform/common Angelica2/AngelicaCommon/Header Angelica2/AngelicaFile/Header
    Angelica2/Angelica3D/Header Angelica2/Output/a3dsdk/include Angelica2/Output/acsdk/include
    Angelica2/Output/amsdk/include Angelica2/AngelicaMedia/Header
    Angelica2/Output/3rdsdk/include)
  target_compile_features(client153_engine_original PRIVATE cxx_std_17)
  target_compile_definitions(client153_engine_original PRIVATE WIN32 NDEBUG _LIB UNICODE _CRT_SECURE_NO_WARNINGS)
  target_compile_options(client153_engine_original PRIVATE -fms-extensions -fshort-wchar -fexceptions
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/platform/common/Windows.h"
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/platform/common/sdk_forward.h"
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/Angelica2/AngelicaCommon/Header/AChar.h"
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/Angelica2/AngelicaCommon/Header/ATime.h")
  target_compile_features(client153_engine_original PRIVATE cxx_std_17)
  target_compile_definitions(client153_engine_original PRIVATE WIN32 NDEBUG _LIB UNICODE _CRT_SECURE_NO_WARNINGS)
  target_compile_options(client153_engine_original PRIVATE -fms-extensions -fshort-wchar -fexceptions
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/platform/common/Windows.h"
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/platform/common/sdk_forward.h"
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/Angelica2/AngelicaCommon/Header/AChar.h"
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/Angelica2/AngelicaCommon/Header/ATime.h")
endif()

# Original GfxCommon component TUs (the model/GFX wrapper layer the game
# links; the sources live under Common/Output/source/Gfx).
if(EMSCRIPTEN)
  add_library(client153_gfx_original OBJECT EXCLUDE_FROM_ALL
    Common/Output/source/Gfx/A3DAreaEmitter.cpp
    Common/Output/source/Gfx/A3DGFXFloatValueTrans.cpp
    Common/Output/source/Gfx/A3DBoxEmitter.cpp
    Common/Output/source/Gfx/A3DCurveEmitter.cpp
    Common/Output/source/Gfx/A3DCylinderEmitter.cpp
    Common/Output/source/Gfx/A3DEllipsoidEmitter.cpp
    Common/Output/source/Gfx/A3DMultiPlaneEmitter.cpp
    Common/Output/source/Gfx/A3DPointEmitter.cpp
    Common/Output/source/Gfx/A3DGFXEx.cpp
    Common/Output/source/Gfx/A3DGFXElement.cpp
    Common/Output/source/Gfx/A3DCombinedAction.cpp
    Common/Output/source/Gfx/A3DGFXExMan.cpp
    Common/Output/source/Gfx/FX_BINDING.cpp
    Common/Output/source/Gfx/EC_Model.cpp
    Common/Output/source/Gfx/EC_ModelHook.cpp
    Common/Output/source/Gfx/EC_ModelMan.cpp
    Common/Output/source/Gfx/EC_ModelLua.cpp
    Common/Output/source/Gfx/GfxEditorUtil.cpp
    Common/Output/source/LuaWrapper/ScriptValue.cpp
    Common/Output/source/LuaWrapper/LuaState.cpp
    Common/Output/source/LuaWrapper/LuaUtil.cpp
    Common/Output/source/LuaWrapper/LuaMemTbl.cpp
    Common/Output/source/LuaWrapper/LuaAPI.cpp
    Common/Output/source/LuaWrapper/LuaDebug.cpp
    Common/Output/source/LuaWrapper/LuaDebugMsg.cpp
    Common/Output/source/LuaWrapper/LuaDebugHelper.cpp
    Common/Output/source/LuaWrapper/LuaDebugIPC.cpp
    Common/Output/source/LuaWrapper/Debugger.cpp
    Common/Output/source/LuaWrapper/LD_NameValue.cpp
    Common/Output/source/LuaWrapper/LuaManager.cpp
    Common/Output/source/LuaWrapper/LuaScript.cpp
    Common/Output/source/Gfx/A3DGFXKeyPoint.cpp
    Common/Output/source/Gfx/A3DGFXInterface.cpp
    Common/Output/source/Gfx/A3DGFXFuncs.cpp
    Common/Output/source/Gfx/A3DGFXModel.cpp
    Common/Output/source/Gfx/A3DGFXModelMan.cpp
    Common/Output/source/Gfx/A3DGFXContainer.cpp
    Common/Output/source/Gfx/A3DGFXECModel.cpp
    Common/Output/source/Gfx/A3DGFXLight.cpp
    Common/Output/source/Gfx/A3DGFXRenderSlot.cpp
    Common/Output/source/Gfx/A3DGFXStreamMan.cpp
    Common/Output/source/Gfx/A3DGFXTexMan.cpp
    Common/Output/source/Gfx/A3DGFXSound.cpp
    Common/Output/source/Gfx/A3DGFXSurfaceRender.cpp
    Common/Output/source/Gfx/A3DGFXEditorInterface.cpp
    Common/Output/source/Gfx/A3DDecalEx.cpp
    Common/Output/source/Gfx/A3DGridDecalEx.cpp
    Common/Output/source/Gfx/A3DGFXRing.cpp
    Common/Output/source/Gfx/A3DGFXShakeCamera.cpp
    Common/Output/source/Gfx/A3DGFXLightning.cpp
    Common/Output/source/Gfx/A3DGFXLightningEx.cpp
    Common/Output/source/Gfx/A3DLtnBolt.cpp
    Common/Output/source/Gfx/A3DLtnTrail.cpp
    Common/Output/source/Gfx/A3DParaboloid.cpp
    Common/Output/source/Gfx/A3DParticleSystemEx.cpp
    Common/Output/source/Gfx/A3DRibbon.cpp
    Common/Output/source/Gfx/A3DTrail.cpp
    Common/Output/source/Gfx/A3DGFXSoundImp.cpp
    Common/Output/source/Gfx/A3DGFXGridAnimation.cpp
    Common/Output/source/Gfx/A3DGFXECMLoader.cpp
    Common/Output/source/Gfx/A3DGFXAnimable.cpp
    Common/Output/source/Gfx/A3DParticleEmitter.cpp
    Common/Output/source/Gfx/TrailList.cpp)
  target_include_directories(client153_gfx_original PRIVATE
    platform/common Angelica2/AngelicaCommon/Header Angelica2/AngelicaFile/Header
    Angelica2/Angelica3D/Header Angelica2/AngelicaMedia/Header
    Angelica2/Output/a3dsdk/include Angelica2/Output/acsdk/include
    Angelica2/Output/afsdk/include Angelica2/Output/amsdk/include
    Angelica2/Output/3rdsdk/include
    Common/Output/include/Gfx Common/Output/include/LuaWrapper
    Common/Output/include/aaudio Common/Output/include/cd
    Common/Output/include/PropDlg Common/Output/include
    Element/Output/include Element/CCommon Element/CElementClient
    Angelica2/Output/3rdsdk/include/lua)
  target_compile_features(client153_gfx_original PRIVATE cxx_std_17)
  target_compile_definitions(client153_gfx_original PRIVATE WIN32 NDEBUG _WINDOWS UNICODE _ANGELICA2 _CRT_SECURE_NO_WARNINGS)
  target_compile_options(client153_gfx_original PRIVATE -fms-extensions -fshort-wchar -fexceptions
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/platform/common/Windows.h"
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/platform/common/sdk_forward.h"
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/Angelica2/AngelicaCommon/Header/AChar.h"
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/Angelica2/AngelicaCommon/Header/ATime.h")
  #	The AUI widget prelude (the probe's forced list): pulling the
  #	adapted AString.h/AArray.h first dedupes the prebuilt SDK copies
  #	(the acsdk headers carry the original's undeclared-member bugs).
  foreach(widget AUIListBox AUIComboBox AUIEditBox AUILabel AUIStillImageButton AUICheckBox AUIImagePicture AUIDialog)
    target_compile_options(client153_gfx_original PRIVATE "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/Common/AUInterface2/${widget}.h")
  endforeach()
endif()

# Build the original UI sources already supported by the browser port.
# Full UI, renderer and client linking remain in progress.
if(EMSCRIPTEN)
  # SpeedTreeRT browser-build boundary: the pinned tree ships the SDK headers
  # and the prebuilt Windows library only (no source), so the referenced
  # CSpeedTreeRT surface is defined once with explicit failure semantics.
  # See src/client153/thirdparty/speedtree/SpeedTreeRT_stub.cpp.
  add_library(client153_speedtree_stub STATIC
    thirdparty/speedtree/SpeedTreeRT_stub.cpp)
  target_include_directories(client153_speedtree_stub PRIVATE
    platform/common Angelica2/AngelicaCommon/Header Angelica2/AngelicaFile/Header
    Angelica2/Angelica3D/Header Angelica2/Output/a3dsdk/include Angelica2/Output/acsdk/include
    Angelica2/Output/amsdk/include Angelica2/AngelicaMedia/Header Angelica2/Output/3rdsdk/include)
  target_compile_features(client153_speedtree_stub PRIVATE cxx_std_17)
  target_compile_options(client153_speedtree_stub PRIVATE -fms-extensions -fshort-wchar -fexceptions
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/platform/common/Windows.h")
  # Official Lua 5.1.5 runtime (MIT). The original 1.5.3 client links
  # Lua 5.1 as a prebuilt lib; the pinned tree ships headers + import
  # lib only, so this restores the same runtime from canonical source.
  # luaconf.h is byte-identical to the vendored 3rdsdk headers, so the
  # default config (no LUA_USE_*) matches the game's TUs.
  set(_lua_src ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/lua-5.1.5/src)
  add_library(client153_lua STATIC
    ${_lua_src}/lapi.c    ${_lua_src}/lcode.c   ${_lua_src}/ldebug.c
    ${_lua_src}/ldo.c     ${_lua_src}/ldump.c   ${_lua_src}/lfunc.c
    ${_lua_src}/lgc.c     ${_lua_src}/llex.c    ${_lua_src}/lmem.c
    ${_lua_src}/lobject.c ${_lua_src}/lopcodes.c ${_lua_src}/lparser.c
    ${_lua_src}/lstate.c  ${_lua_src}/lstring.c ${_lua_src}/ltable.c
    ${_lua_src}/ltm.c     ${_lua_src}/lundump.c ${_lua_src}/lvm.c
    ${_lua_src}/lzio.c    ${_lua_src}/lauxlib.c ${_lua_src}/lbaselib.c
    ${_lua_src}/ldblib.c  ${_lua_src}/liolib.c  ${_lua_src}/lmathlib.c
    ${_lua_src}/loslib.c  ${_lua_src}/ltablib.c ${_lua_src}/lstrlib.c
    ${_lua_src}/linit.c   ${_lua_src}/loadlib.c)
  target_include_directories(client153_lua PUBLIC ${_lua_src})
  set_target_properties(client153_lua PROPERTIES
    LINKER_LANGUAGE C
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/client153)
  target_compile_options(client153_lua PRIVATE -std=gnu89 -O2)

  # Proprietary-middleware browser-build boundaries: FMOD (the AudioEngine
  # wrapper) and TeamSpeak ship headers + prebuilt Windows libraries only in
  # the pinned tree; the defence (anti-cheat) API cannot run in a browser
  # sandbox. The referenced entry points are defined once with explicit
  # failure semantics. See thirdparty/fmod/FAudioEngine_stub.cpp,
  # thirdparty/teamspeak/clientlib_stub.cpp and
  # thirdparty/defence/defence_stub.cpp.
  add_library(client153_middleware_stub STATIC
    thirdparty/fmod/FAudioEngine_stub.cpp
    thirdparty/teamspeak/clientlib_stub.cpp
    thirdparty/defence/defence_stub.cpp
    thirdparty/immersion/ifc_stub.cpp
    thirdparty/xiph/vorbisfile_stub.cpp
    thirdparty/mpg123/mpg123_stub.cpp
    thirdparty/ftdriver/ftdriver_stub.cpp)
  target_include_directories(client153_middleware_stub PRIVATE
    platform/common Angelica2/AngelicaCommon/Header Angelica2/AngelicaFile/Header
    Angelica2/Angelica3D/Header Angelica2/Output/a3dsdk/include Angelica2/Output/acsdk/include
    Angelica2/Output/amsdk/include Angelica2/AngelicaMedia/Header Angelica2/Output/3rdsdk/include
    Common/Output/include/aaudio Element/Output/include Element/CElementClient/defence
    Element/CElementClient/Network/IOLib Common/AUInterface2)
  target_compile_features(client153_middleware_stub PRIVATE cxx_std_17)
  target_compile_options(client153_middleware_stub PRIVATE -fms-extensions -fshort-wchar -fexceptions
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/platform/common/Windows.h")
  # Original game-adjacent TUs outside ElementClient.vcproj: the skill data
  # layer, the AutoMove pathfinding agent and the CHBasedCD collision core.
  # They compile with the game-project environment (the same include set and
  # definitions the probe uses for the game project).
  add_library(client153_game_original OBJECT EXCLUDE_FROM_ALL
    Element/CFWEditorLib/CharContour.cpp
    Element/CFWEditorLib/CharCurve.cpp
    Element/CFWEditorLib/CharOutline.cpp
    Element/CFWEditorLib/CharVerticesBuffer.cpp
    Element/CFWEditorLib/GLUPolygon.cpp
    Element/CFWEditorLib/GLUTess.cpp
    Element/CFWEditorLib/GLUContour.cpp
    Element/CFWEditorLib/FWFormula.cpp
    Element/CFWEditorLib/FWFormulaElement.cpp
    Element/CFWEditorLib/FWOperand.cpp
    Element/CFWEditorLib/FWOperator.cpp
    Element/CFWEditorLib/FWArt.cpp
    Element/CFWEditorLib/FWModifier.cpp
    Element/CFWEditorLib/FWArtArc.cpp
    Element/CFWEditorLib/FWArtDoubleArc.cpp
    Element/CFWEditorLib/FWArtNoChange.cpp
    Element/CFWEditorLib/FWArtStepScale.cpp
    Element/CFWEditorLib/FWArtTilt.cpp
    Element/CFWEditorLib/FWArtWave.cpp
    Element/CFWEditorLib/FWModifierParabola.cpp
    Element/CFWEditorLib/FWModifierRing.cpp
    Element/CFWEditorLib/FWObject.cpp
    Element/CElementSkill/ElementSkill.cpp
    Element/AutoPFImp/AutoMoveImp/MoveAgent.cpp
    Common/CHBasedCD/ConvexHullData.cpp
    Common/CHBasedCD/aabbcd.cpp
    Common/CHBasedCD/HalfSpace.cpp
    Common/CHBasedCD/CDWithCH.cpp
    Common/CHBasedCD/Face.cpp
    Element/CElementSkill/skill.cpp
    Element/CElementSkill/skillwrapper.cpp
    Element/AutoPFImp/AutoMoveImp/PfClu.cpp
    Element/AutoPFImp/AutoMoveImp/PfConstant.cpp
    Element/AutoPFImp/AutoMoveImp/MoveMap.cpp
    Element/AutoPFImp/AutoMoveImp/LayerMap.cpp
    Element/AutoPFImp/AutoMoveImp/Island.cpp
    Element/AutoPFImp/AutoMoveImp/OptimizePath.cpp
    Element/AutoPFImp/AutoMoveImp/Graph.cpp
    Element/AutoPFImp/AutoMoveImp/GHeap.cpp
    Element/AutoPFImp/AutoMoveImp/MapNode.cpp
    Element/AutoPFImp/AutoMoveImp/MapGraph.cpp
    Element/AutoPFImp/AutoMoveImp/GAStar.cpp
    Element/AutoPFImp/AutoMoveImp/MAStar.cpp
    Element/AutoPFImp/AutoMoveImp/CluCommon.cpp
    Element/AutoPFImp/AutoMoveImp/CluGraph.cpp
    Element/AutoPFImp/AutoMoveImp/ClusterAbstraction.cpp
    Element/AutoPFImp/AutoMoveImp/CluGraphConnector.cpp
    Element/AutoPFImp/AutoMoveImp/MultiCluGraph.cpp
    Common/CHBasedCD/Matrix.cpp
    Common/CHBasedCD/LEquations.cpp
    Common/Output/source/Gfx/RandStringContainer.cpp
    Element/CFWEditorLib/FWGlyphGraphBase.cpp
    Element/CFWEditorLib/FWGlyphPolygon.cpp
    Element/CFWEditorLib/FWGlyphBezier.cpp
    Element/CFWEditorLib/FWPlayer.cpp
    Element/CFWEditorLib/FWState.cpp
    Element/CFWEditorLib/FWDialogBase.cpp
    Element/CFWEditorLib/FWDialogBezierMenu.cpp
    Element/CFWEditorLib/FWDialogBoard.cpp
    Element/CFWEditorLib/FWDialogCustom.cpp
    Element/CFWEditorLib/FWDialogEdit.cpp
    Element/CFWEditorLib/FWDialogGraph.cpp
    Element/CFWEditorLib/FWDialogGraphMenu.cpp
    Element/CFWEditorLib/FWDialogLanch.cpp
    Element/CFWEditorLib/FWDialogMain.cpp
    Element/CFWEditorLib/FWDialogParam.cpp
    Element/CFWEditorLib/FWDialogPlay.cpp
    Element/CFWEditorLib/FWDialogPolygonMenu.cpp
    Element/CFWEditorLib/FWDialogScale.cpp
    Element/CFWEditorLib/FWDialogSimple.cpp
    Element/CFWEditorLib/FWDialogText.cpp
    Element/CFWEditorLib/FWDialogTool.cpp
    Element/CFWEditorLib/FWStateSelect.cpp
    Element/CFWEditorLib/FWCommandSetGlyphProp.cpp
    Element/CFWEditorLib/FWOutterProperty.cpp
    Element/CFWEditorLib/FWAUIManager.cpp
    Element/CFWEditorLib/FWCommandList.cpp
    Element/CFWEditorLib/FWFormulaList.cpp
    Element/CFWEditorLib/FWGfxWithModifier.cpp
    Element/CFWEditorLib/FWGlyphComposite.cpp
    Element/CFWEditorLib/FWFirework.cpp
    Element/CFWEditorLib/FWStream.cpp
    Element/CFWEditorLib/FWRuntimeClass.cpp
    Element/CFWEditorLib/FWAlgorithm.cpp
    Element/CFWEditorLib/FWGlyphRectBase.cpp
    Element/CFWEditorLib/FWFireworkLeaf.cpp
    Element/CFWEditorLib/FWGlyph.cpp
    Element/CFWEditorLib/FWFlatCollector.cpp
    Element/CFWEditorLib/FWManager.cpp
    Element/CFWEditorLib/FWTemplate.cpp
    Element/CFWEditorLib/FWAssemblySet.cpp
    Element/CFWEditorLib/FWView.cpp
    Element/CFWEditorLib/FWDoc.cpp
    Element/CFWEditorLib/FWArchive.cpp
    Element/CFWEditorLib/FWFontEnum.cpp
    Element/CFWEditorLib/FWGlyphText.cpp
    Element/CFWEditorLib/FWGlyphList.cpp
    Element/CFWEditorLib/FWGlyphFirework.cpp
    Element/CFWEditorLib/FWFireworkLanch.cpp
    Element/CFWEditorLib/FWInnerProperty.cpp
    Element/CFWEditorLib/FWAssemblyCache.cpp
    Element/CFWEditorLib/FWAssemblyProfile.cpp
    Element/CFWEditorLib/FWModifierCreator.cpp
    Element/CFWEditorLib/FWArtCreator.cpp
    Element/CFWEditorLib/FWFireworkCustom.cpp
    Element/CFWEditorLib/FWFireworkGraph.cpp
    Element/CFWEditorLib/FWFireworkSimple.cpp
    Element/CFWEditorLib/FWFireworkElement.cpp
    Element/CFWEditorLib/FWLog.cpp
    Common/CHBasedCD/BestFit.cpp)
  target_include_directories(client153_game_original PRIVATE
    platform/common Angelica2/AngelicaCommon/Header Angelica2/AngelicaFile/Header
    Angelica2/Angelica3D/Header Angelica2/AngelicaMedia/Header
    Angelica2/Output/a3dsdk/include Angelica2/Output/acsdk/include
    Angelica2/Output/afsdk/include Angelica2/Output/amsdk/include
    Angelica2/Output/3rdsdk/include Angelica2/Output/3rdsdk/include/lua
    Common/Output/include/Gfx Common/Output/include/LuaWrapper
    Common/Output/include/aaudio Common/Output/include/cd
    Common/Output/include/PropDlg Common/Output/include
    Element/Output/include Element/CCommon Element/CElementClient
    Element/Output/include/fw Element/CElementSkill Element/AutoPFImp/AutoMoveImp Common/CHBasedCD
    Common/AUInterface2)
  target_compile_features(client153_game_original PRIVATE cxx_std_17)
  target_compile_definitions(client153_game_original PRIVATE WIN32 NDEBUG _WINDOWS UNICODE _ANGELICA2 _CRT_SECURE_NO_WARNINGS _FW_EDITOR)
  target_compile_options(client153_game_original PRIVATE -fms-extensions -fshort-wchar -fexceptions
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/platform/common/Windows.h"
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/platform/common/sdk_forward.h"
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/Angelica2/AngelicaCommon/Header/AChar.h"
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/Angelica2/AngelicaCommon/Header/ATime.h")
  foreach(widget AUIListBox AUIComboBox AUIEditBox AUILabel AUIStillImageButton AUICheckBox AUIImagePicture AUIDialog)
    target_compile_options(client153_game_original PRIVATE "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/Common/AUInterface2/${widget}.h")
  endforeach()
endif()

# Build the original UI sources already supported by the browser port.
# Full UI, renderer and client linking remain in progress.
if(EMSCRIPTEN)
  add_library(client153_aui_original OBJECT EXCLUDE_FROM_ALL
    Common/AUInterface2/AUICheckBox.cpp
    Common/AUInterface2/AUIConsole.cpp
    Common/AUInterface2/AUILabel.cpp
    Common/AUInterface2/AUIModelPicture.cpp
    Common/AUInterface2/A3DFTFont.cpp
    Common/AUInterface2/AUIDialog.cpp
    Common/AUInterface2/AUIManager.cpp
    Common/AUInterface2/AUIPicture.cpp
    Common/AUInterface2/AUIRadioButton.cpp
    Common/AUInterface2/A2DDotLine.cpp
    Common/AUInterface2/A2DDotLineMan.cpp
    Common/AUInterface2/A3DAlterableSkinModel.cpp
    Common/AUInterface2/AUI.cpp
    Common/AUInterface2/AUICTranslate.cpp
    Common/AUInterface2/AUIFrame.cpp
    Common/AUInterface2/CSplit.cpp
    Common/AUInterface2/AUICommon.cpp
    Common/AUInterface2/AUIObject.cpp
    Common/AUInterface2/AUIComboBox.cpp
    Common/AUInterface2/AUIStillImageButton.cpp
    Common/AUInterface2/AUIScroll.cpp
    Common/AUInterface2/AUISlider.cpp
    Common/AUInterface2/AUISubDialog.cpp
    Common/AUInterface2/AUIVerticalText.cpp
    Common/AUInterface2/AUITreeView.cpp
    Common/AUInterface2/AXMLFile.cpp)
  target_include_directories(client153_aui_original PRIVATE
    Angelica2/AngelicaCommon/Header Angelica2/AngelicaFile/Header Angelica2/Angelica3D/Header
    platform/common Common/AUInterface2 Common/Output/include/PropDlg Common/Output/include/Gfx
    Common/Output/include/LuaWrapper Common/Output/include/aaudio Angelica2/Output/3rdsdk/include/lua
    Angelica2/Output/3rdsdk/include Angelica2/Output/a3dsdk/include Angelica2/Output/acsdk/include
    Angelica2/Output/afsdk/include Angelica2/Output/amsdk/include)
  target_compile_features(client153_aui_original PRIVATE cxx_std_17)
  target_compile_definitions(client153_aui_original PRIVATE WIN32 NDEBUG _LIB UNICODE _ANGELICA2 _CRT_SECURE_NO_WARNINGS)
  target_compile_options(client153_aui_original PRIVATE -fms-extensions -fshort-wchar -fexceptions
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/platform/common/Windows.h"
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/platform/common/sdk_forward.h"
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/Angelica2/AngelicaCommon/Header/AChar.h"
    "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/Angelica2/AngelicaCommon/Header/ATime.h")
  foreach(widget AUIListBox AUIComboBox AUIEditBox AUILabel AUIStillImageButton AUICheckBox AUIImagePicture AUIDialog)
    target_compile_options(client153_aui_original PRIVATE "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/Common/AUInterface2/${widget}.h")
  endforeach()
endif()

add_library(client153_formats STATIC
  Angelica2/Angelica3D/port/mox.h
  Angelica2/Angelica3D/port/dds.h
  Angelica2/Angelica3D/port/mox.cpp
  Angelica2/Angelica3D/port/dds.cpp
  Angelica2/Angelica3D/port/ski.cpp)
target_include_directories(client153_formats PUBLIC Angelica2/Angelica3D/port)
target_compile_features(client153_formats PUBLIC cxx_std_17)
target_compile_options(client153_formats PRIVATE -Wall -Wextra -Werror -pedantic "$<$<CXX_COMPILER_ID:GNU>:-Wno-template-body>")
if(EMSCRIPTEN)
  target_compile_options(client153_formats PUBLIC -fexceptions)
  target_link_options(client153_formats INTERFACE -fexceptions)
  add_library(client153_webgl_caps STATIC platform/web/webgl_caps.cpp)
  target_compile_features(client153_webgl_caps PUBLIC cxx_std_17)
  add_executable(client153 platform/web/runtime.cpp)
  target_link_libraries(client153 PRIVATE client153_formats client153_file client153_math client153_webgl_caps)
  target_compile_options(client153 PRIVATE -Wall -Wextra -Werror "$<$<CXX_COMPILER_ID:GNU>:-Wno-template-body>")
  set_target_properties(client153 PROPERTIES CXX_EXTENSIONS ON)
  target_link_options(client153 PRIVATE
    --no-entry
    "-sMIN_WEBGL_VERSION=2" "-sMAX_WEBGL_VERSION=2"
    "-sMODULARIZE=1" "-sEXPORT_ES6=1" "-sENVIRONMENT=web"
    "-sFILESYSTEM=1" "-sALLOW_MEMORY_GROWTH=1"
    "-sEXPORTED_FUNCTIONS=[_malloc,_free,_client_init,_client_load,_client_load_file,_client_texture,_client_texture_name,_client_texture_name_size,_client_mesh_count,_client_draw,_client_error,_client_dispose,_pw153_set_cursor_position,_pw153_set_canvas_origin]"
    "-sEXPORTED_RUNTIME_METHODS=[UTF8ToString,HEAPU8,FS]")
  set_target_properties(client153 PROPERTIES SUFFIX ".mjs" RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/browser")
  configure_file("${PROJECT_SOURCE_DIR}/web/client153/index.html" "${CMAKE_BINARY_DIR}/browser/index.html" COPYONLY)
  configure_file("${PROJECT_SOURCE_DIR}/web/client153/app.mjs" "${CMAKE_BINARY_DIR}/browser/app.mjs" COPYONLY)
endif()

add_library(client153_file STATIC
  Angelica2/AngelicaFile/Source/AFile.cpp
  Angelica2/AngelicaFile/Source/AFI.cpp
  Angelica2/AngelicaFile/Source/AFilePackBase.cpp
  Angelica2/AngelicaFile/Source/afilepackage.cpp
  Angelica2/AngelicaFile/Source/AFilePackMan.cpp
  Angelica2/AngelicaFile/Source/AFilePackGame.cpp
  Angelica2/AngelicaFile/Source/AFPCKConstants.cpp
  Angelica2/AngelicaFile/Source/AFileImage.cpp
  Angelica2/AngelicaFile/Source/AIniFile.cpp
  Angelica2/AngelicaFile/Source/AScriptFile.cpp
  Angelica2/AngelicaCommon/Source/ATempMemMan.cpp)
target_include_directories(client153_file PUBLIC Angelica2/AngelicaFile/Header)
target_link_libraries(client153_file PUBLIC client153_common client153_platform)
target_link_libraries(client153_file PUBLIC ${PW153_ZLIB_LINK})
#	Some legacy path operations still require review for truncation; keep
#	the GCC diagnostic visible while porting those call sites.
target_compile_options(client153_file PRIVATE -Wall -Wextra -Werror -pedantic "$<$<CXX_COMPILER_ID:GNU>:-Wno-template-body>" "$<$<CXX_COMPILER_ID:GNU>:-Wno-error=format-truncation>" "$<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wno-error=keyword-macro>")

# Original matrix/vector unions retain MSVC anonymous-struct layout.
add_library(client153_math STATIC Angelica2/Angelica3D/Source/A3DMatrix.cpp Angelica2/Angelica3D/Source/A3DQuaternion.cpp)
target_link_libraries(client153_math PUBLIC client153_common)
target_include_directories(client153_math PUBLIC Angelica2/Angelica3D/Header)
target_compile_features(client153_math PUBLIC cxx_std_17)
target_compile_options(client153_math PRIVATE -Wall -Wextra -Werror "$<$<CXX_COMPILER_ID:GNU>:-Wno-template-body>")

set_target_properties(client153_math PROPERTIES CXX_EXTENSIONS ON)

add_library(client153_animation STATIC Angelica2/Angelica3D/Source/A3DTrackData.cpp)
target_link_libraries(client153_animation PUBLIC client153_math client153_file)
target_compile_options(client153_animation PRIVATE -Wall -Wextra -Werror "$<$<CXX_COMPILER_ID:GNU>:-Wno-template-body>")

target_sources(client153_math PRIVATE
  Angelica2/Angelica3D/Header/A3DObject.h
  Angelica2/Angelica3D/Header/A3DClassID.h)

# Original skeleton support: PI globals, scalar math helpers, geometry bounds,
# coordinate spaces, controllers and IK, on the portable platform clock.
add_library(client153_skeleton STATIC
  Angelica2/Angelica3D/Source/A3DConfig.cpp
  Angelica2/Angelica3D/Source/A3DJoint.cpp
  Angelica2/Angelica3D/Source/A3DBone.cpp
  Angelica2/Angelica3D/Source/A3DBoneController.cpp
  Angelica2/Angelica3D/Source/A3DTrackMan.cpp
  Angelica2/Angelica3D/Source/A3DSkeleton.cpp
  Angelica2/Angelica3D/Source/A3DSkinModelActionCore.cpp
  Angelica2/Angelica3D/Source/A3DPI.cpp
  Angelica2/Angelica3D/Source/A3DFuncs.cpp
  Angelica2/Angelica3D/Source/A3DGeometry.cpp
  Angelica2/Angelica3D/Source/A3DController.cpp
  Angelica2/Angelica3D/Source/A3DCoordinate.cpp
  Angelica2/Angelica3D/Source/A3DIKData.cpp
  Angelica2/Angelica3D/Source/A3DIKSolver.cpp)
target_link_libraries(client153_skeleton PUBLIC client153_math client153_animation client153_platform)
target_include_directories(client153_skeleton PUBLIC Angelica2/Angelica3D/Header)
target_compile_features(client153_skeleton PUBLIC cxx_std_17)
#	-pedantic omitted like client153_math: the original vector/matrix unions use MSVC anonymous-struct layout.
target_compile_options(client153_skeleton PRIVATE -Wall -Wextra -Werror "$<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wno-error=keyword-macro>")
set_target_properties(client153_skeleton PROPERTIES CXX_EXTENSIONS ON)

add_library(client153_skinning STATIC Angelica2/Angelica3D/port/skin_binding.cpp Angelica2/Angelica3D/port/animated_skin.cpp)
target_link_libraries(client153_skinning PUBLIC client153_skeleton client153_formats)
target_compile_options(client153_skinning PRIVATE -Wall -Wextra -Werror "$<$<CXX_COMPILER_ID:GNU>:-Wno-template-body>")

# Diagnostic only: attempt to link all currently compiled original engine and
# UI objects. Unresolved symbols fail the target; this is not a game binary.
if(EMSCRIPTEN)
  add_custom_target(client153_original_link_probe
    COMMAND ${CMAKE_CXX_COMPILER} --no-entry -fexceptions -sUSE_ZLIB=1
      -Wl,--no-gc-sections -Wl,--error-limit=20
      $<TARGET_OBJECTS:client153_aui_original>
      $<TARGET_OBJECTS:client153_engine_original>
      $<TARGET_FILE:client153_skeleton> $<TARGET_FILE:client153_animation>
      $<TARGET_FILE:client153_file> $<TARGET_FILE:client153_math>
      $<TARGET_FILE:client153_common> $<TARGET_FILE:client153_platform>
      -o ${CMAKE_CURRENT_BINARY_DIR}/client153-original-link-probe.js
    DEPENDS client153_aui_original client153_engine_original client153_skeleton
      client153_animation client153_file client153_math client153_common client153_platform
    COMMAND_EXPAND_LISTS VERBATIM)
endif()
