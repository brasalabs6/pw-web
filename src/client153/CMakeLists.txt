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

add_library(client153_platform STATIC platform/common/clock.cpp platform/common/format.cpp platform/common/utf16.cpp platform/common/widestr.cpp platform/common/keystate.cpp platform/common/findfiles.cpp platform/common/toolhelp.cpp platform/common/atlcom_registry.cpp platform/common/inifile.cpp)
target_include_directories(client153_platform PUBLIC platform/common)
target_include_directories(client153_platform PUBLIC Angelica2/AngelicaCommon/Header)
target_compile_features(client153_platform PUBLIC cxx_std_17)
target_compile_options(client153_platform PRIVATE -Wall -Wextra -Werror -pedantic)
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
target_compile_options(client153_common PRIVATE -Wall -Wextra -Werror -pedantic "$<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wno-error=keyword-macro>")
if(EMSCRIPTEN)
  target_compile_options(client153_platform PUBLIC -fexceptions)
  target_link_options(client153_platform INTERFACE -fexceptions)
  target_compile_options(client153_common PUBLIC -fexceptions)
  target_link_options(client153_common INTERFACE -fexceptions)
endif()

# Original engine compilation target; linking the complete renderer is still pending.
if(EMSCRIPTEN)
  add_library(client153_engine_original OBJECT EXCLUDE_FROM_ALL
    Angelica2/Angelica3D/Source/A3DCamera.cpp
    Angelica2/Angelica3D/Source/A3DCameraBase.cpp
    Angelica2/Angelica3D/Source/A3DOrthoCamera.cpp
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
    Angelica2/Angelica3D/Source/A3DEngine.cpp
    Angelica2/Angelica3D/Source/A3DTextureMan.cpp
    Angelica2/Angelica3D/Source/A3DShader.cpp
    Angelica2/Angelica3D/Source/A3DShaderMan.cpp
    Angelica2/Angelica3D/Source/A3DControl.cpp
    Angelica2/AngelicaMedia/Source/AMSoundBufferMan.cpp
    Angelica2/AngelicaMedia/Source/AMConfig.cpp
    Angelica2/AngelicaMedia/Source/AMPI.cpp
    Angelica2/AngelicaMedia/Source/AMEngine.cpp
    Angelica2/AngelicaMedia/Source/AMSoundEngine.cpp)
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
    Common/Output/source/Gfx/A3DGFXEx.cpp
    Common/Output/source/Gfx/A3DCombinedAction.cpp
    Common/Output/source/Gfx/A3DGFXExMan.cpp
    Common/Output/source/Gfx/EC_Model.cpp
    Common/Output/source/Gfx/EC_ModelHook.cpp
    Common/Output/source/Gfx/EC_ModelMan.cpp
    Common/Output/source/Gfx/EC_ModelLua.cpp
    Common/Output/source/Gfx/GfxEditorUtil.cpp
    Common/Output/source/LuaWrapper/ScriptValue.cpp)
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
target_compile_options(client153_formats PRIVATE -Wall -Wextra -Werror -pedantic)
if(EMSCRIPTEN)
  target_compile_options(client153_formats PUBLIC -fexceptions)
  target_link_options(client153_formats INTERFACE -fexceptions)
  add_library(client153_webgl_caps STATIC platform/web/webgl_caps.cpp)
  target_compile_features(client153_webgl_caps PUBLIC cxx_std_17)
  add_executable(client153 platform/web/runtime.cpp)
  target_link_libraries(client153 PRIVATE client153_formats client153_file client153_math client153_webgl_caps)
  target_compile_options(client153 PRIVATE -Wall -Wextra -Werror)
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
target_compile_options(client153_file PRIVATE -Wall -Wextra -Werror -pedantic "$<$<CXX_COMPILER_ID:GNU>:-Wno-error=format-truncation>" "$<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wno-error=keyword-macro>")

# Original matrix/vector unions retain MSVC anonymous-struct layout.
add_library(client153_math STATIC Angelica2/Angelica3D/Source/A3DMatrix.cpp Angelica2/Angelica3D/Source/A3DQuaternion.cpp)
target_link_libraries(client153_math PUBLIC client153_common)
target_include_directories(client153_math PUBLIC Angelica2/Angelica3D/Header)
target_compile_features(client153_math PUBLIC cxx_std_17)
target_compile_options(client153_math PRIVATE -Wall -Wextra -Werror)

set_target_properties(client153_math PROPERTIES CXX_EXTENSIONS ON)

add_library(client153_animation STATIC Angelica2/Angelica3D/Source/A3DTrackData.cpp)
target_link_libraries(client153_animation PUBLIC client153_math client153_file)
target_compile_options(client153_animation PRIVATE -Wall -Wextra -Werror)

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
target_compile_options(client153_skinning PRIVATE -Wall -Wextra -Werror)

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
