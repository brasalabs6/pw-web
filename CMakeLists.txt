cmake_minimum_required(VERSION 3.25)
project(pw_web_client153 LANGUAGES C CXX)
include(CTest)
#	Emscripten's zlib port must be requested for every compile and link,
#	includings test executables defined in this file.
#	The original client is a UTF-16 (MSVC wchar_t = 16-bit) build; every
#	port target and test compiles with the same width.
add_compile_options(-fshort-wchar)

if(EMSCRIPTEN)
  add_compile_options(-sUSE_ZLIB=1)
  add_link_options(-sUSE_ZLIB=1)
endif()

add_subdirectory(src/client153)
if(BUILD_TESTING)
  if(EMSCRIPTEN)
    add_executable(client153_ftdriver_original_probe tests/client153/ftdriver_original_probe.cpp)
    target_link_libraries(client153_ftdriver_original_probe PRIVATE client153_ftdriver_original)
    target_compile_options(client153_ftdriver_original_probe PRIVATE -fms-extensions -fexceptions
      "SHELL:-include ${PROJECT_SOURCE_DIR}/src/client153/platform/common/Windows.h")
    target_link_options(client153_ftdriver_original_probe PRIVATE -fexceptions "-sENVIRONMENT=node" "-sNODERAWFS=1")
    add_executable(client153_split_wasm_test tests/client153/split_wasm.cpp
      src/client153/Common/AUInterface2/CSplit.cpp)
    target_include_directories(client153_split_wasm_test PRIVATE
      $<TARGET_PROPERTY:client153_aui_original,INCLUDE_DIRECTORIES>)
    target_compile_definitions(client153_split_wasm_test PRIVATE
      $<TARGET_PROPERTY:client153_aui_original,COMPILE_DEFINITIONS>)
    target_compile_options(client153_split_wasm_test PRIVATE
      $<TARGET_PROPERTY:client153_aui_original,COMPILE_OPTIONS>)
    target_link_options(client153_split_wasm_test PRIVATE -fexceptions "-sENVIRONMENT=node")
    target_link_libraries(client153_split_wasm_test PRIVATE client153_common)
    add_test(NAME client153_split_wasm COMMAND client153_split_wasm_test)
    add_executable(client153_gnet_web_scheduler_test tests/client153/gnet_web_scheduler.cpp)
    target_include_directories(client153_gnet_web_scheduler_test PRIVATE src/client153/platform/common)
    target_compile_features(client153_gnet_web_scheduler_test PRIVATE cxx_std_17)
    target_link_options(client153_gnet_web_scheduler_test PRIVATE "-sENVIRONMENT=node")
    add_test(NAME client153_gnet_web_scheduler COMMAND client153_gnet_web_scheduler_test)
    set_tests_properties(client153_gnet_web_scheduler PROPERTIES PASS_REGULAR_EXPRESSION "web poll callbacks stopped")
    add_executable(client153_readonly_package_test tests/client153/readonly_package.c)
    target_link_options(client153_readonly_package_test PRIVATE "-sENVIRONMENT=node" "-sFORCE_FILESYSTEM=1"
      "SHELL:--pre-js ${PROJECT_SOURCE_DIR}/scripts/client153-readonly-node.js"
      "SHELL:--pre-js ${PROJECT_SOURCE_DIR}/tests/client153/readonly_package_pre.js")
    add_test(NAME client153_readonly_package COMMAND client153_readonly_package_test)
    set_tests_properties(client153_readonly_package PROPERTIES
      ENVIRONMENT "PW153_FIXTURE=${PROJECT_SOURCE_DIR}/tests/client153/readonly_fixture.txt")
    add_executable(client153_wide_length_optimized_test
      tests/client153/wide_length_optimized.cpp src/client153/platform/common/widestr.cpp)
    target_include_directories(client153_wide_length_optimized_test PRIVATE src/client153/platform/common)
    target_compile_options(client153_wide_length_optimized_test PRIVATE -O3 -fno-builtin-wcslen)
    target_link_options(client153_wide_length_optimized_test PRIVATE -O3 "-sENVIRONMENT=node")
    add_test(NAME client153_wide_length_optimized COMMAND client153_wide_length_optimized_test)
    add_executable(client153_glu_tess_test tests/client153/glu_tessellation.c)
    target_link_libraries(client153_glu_tess_test PRIVATE client153_glu_tess)
    target_link_options(client153_glu_tess_test PRIVATE "-sENVIRONMENT=node")
    add_test(NAME client153_glu_tess COMMAND client153_glu_tess_test)
    add_executable(client153_mesh_shrink_test
      tests/client153/mesh_shrink.cpp
      src/client153/Element/CFWEditorLib/Mesh.cpp
      src/client153/Element/CFWEditorLib/FWTextMesh.cpp
      src/client153/Element/CFWEditorLib/Triangle.cpp)
    target_include_directories(client153_mesh_shrink_test PRIVATE
      src/client153/Element/CFWEditorLib $<TARGET_PROPERTY:client153_game_original,INCLUDE_DIRECTORIES>)
    target_compile_definitions(client153_mesh_shrink_test PRIVATE
      $<TARGET_PROPERTY:client153_game_original,COMPILE_DEFINITIONS>)
    target_compile_options(client153_mesh_shrink_test PRIVATE
      $<TARGET_PROPERTY:client153_game_original,COMPILE_OPTIONS>
      -fsanitize=undefined -fno-sanitize-recover=undefined)
    target_link_options(client153_mesh_shrink_test PRIVATE
      -fexceptions -fsanitize=undefined -fno-sanitize-recover=undefined "-sENVIRONMENT=node")
    target_link_libraries(client153_mesh_shrink_test PRIVATE client153_skeleton client153_file)
    add_test(NAME client153_mesh_shrink COMMAND client153_mesh_shrink_test)
    add_executable(client153_contour_simplify_test
      tests/client153/contour_simplify.cpp
      src/client153/Element/CFWEditorLib/CharContour.cpp
      src/client153/Element/CFWEditorLib/CharVerticesBuffer.cpp)
    target_include_directories(client153_contour_simplify_test PRIVATE
      src/client153/Element/CFWEditorLib $<TARGET_PROPERTY:client153_game_original,INCLUDE_DIRECTORIES>)
    target_compile_definitions(client153_contour_simplify_test PRIVATE
      $<TARGET_PROPERTY:client153_game_original,COMPILE_DEFINITIONS>)
    target_compile_options(client153_contour_simplify_test PRIVATE
      $<TARGET_PROPERTY:client153_game_original,COMPILE_OPTIONS>
      -fsanitize=undefined -fno-sanitize-recover=undefined)
    target_link_options(client153_contour_simplify_test PRIVATE
      -fexceptions -fsanitize=undefined -fno-sanitize-recover=undefined "-sENVIRONMENT=node")
    target_link_libraries(client153_contour_simplify_test PRIVATE client153_skeleton client153_file)
    add_test(NAME client153_contour_simplify COMMAND client153_contour_simplify_test)
    add_executable(client153_webgl_caps_test tests/client153/webgl_caps.cpp)
    add_executable(client153_wasm_pointer_bounds_test tests/client153/wasm_pointer_bounds.cpp)
    target_link_libraries(client153_wasm_pointer_bounds_test PRIVATE client153_platform)
    target_link_options(client153_wasm_pointer_bounds_test PRIVATE "-sENVIRONMENT=node")
    add_test(NAME client153_wasm_pointer_bounds COMMAND node $<TARGET_FILE:client153_wasm_pointer_bounds_test>)
    target_include_directories(client153_webgl_caps_test PRIVATE src/client153/platform/web)
    target_link_libraries(client153_webgl_caps_test PRIVATE client153_webgl_caps)
    target_link_options(client153_webgl_caps_test PRIVATE "-sMIN_WEBGL_VERSION=2" "-sMAX_WEBGL_VERSION=2" -fexceptions)
    target_compile_options(client153_webgl_caps_test PRIVATE -fexceptions)
    add_executable(client153_d3d8_factory_browser_test tests/client153/d3d8_factory_browser.cpp)
    add_executable(client153_image_surface_browser_test tests/client153/d3d8_factory_browser.cpp)
    target_compile_definitions(client153_image_surface_browser_test PRIVATE PW153_IMAGE_SURFACE_ONLY)
    add_executable(client153_d3dx_image_decode_test tests/client153/d3dx_image_decode.cpp)
    target_link_libraries(client153_d3dx_image_decode_test PRIVATE client153_webgl_caps)
    target_include_directories(client153_d3dx_image_decode_test PRIVATE src/client153/platform/web src/client153/platform/common src/client153/Angelica2/AngelicaCommon/Header src/client153/Angelica2/Angelica3D/Header)
    target_link_options(client153_d3dx_image_decode_test PRIVATE "-sMIN_WEBGL_VERSION=2" "-sMAX_WEBGL_VERSION=2")
    target_include_directories(client153_image_surface_browser_test PRIVATE src/client153/platform/web src/client153/platform/common src/client153/Angelica2/AngelicaCommon/Header src/client153/Angelica2/Angelica3D/Header)
    target_link_libraries(client153_image_surface_browser_test PRIVATE client153_webgl_caps)
    target_link_options(client153_image_surface_browser_test PRIVATE "-sMIN_WEBGL_VERSION=2" "-sMAX_WEBGL_VERSION=2")
    add_executable(client153_gdi_text_browser_test tests/client153/gdi_text_browser.cpp)
    target_link_libraries(client153_gdi_text_browser_test PRIVATE client153_platform)
    target_include_directories(client153_d3d8_factory_browser_test PRIVATE src/client153/platform/web src/client153/platform/common src/client153/Angelica2/AngelicaCommon/Header src/client153/Angelica2/Angelica3D/Header)
    target_link_libraries(client153_d3d8_factory_browser_test PRIVATE client153_webgl_caps)
    target_link_options(client153_d3d8_factory_browser_test PRIVATE "-sMIN_WEBGL_VERSION=2" "-sMAX_WEBGL_VERSION=2")
    add_executable(client153_aui_xml_test tests/client153/aui_xml.cpp src/client153/Common/AUInterface2/AXMLFile.cpp)
    target_include_directories(client153_aui_xml_test PRIVATE
      src/client153/Angelica2/AngelicaCommon/Header src/client153/Angelica2/AngelicaFile/Header
      src/client153/Angelica2/Angelica3D/Header src/client153/platform/common
      src/client153/Common/AUInterface2 src/client153/Common/Output/include/PropDlg
      src/client153/Common/Output/include/Gfx src/client153/Common/Output/include/LuaWrapper
      src/client153/Common/Output/include/aaudio src/client153/Angelica2/Output/3rdsdk/include/lua
      src/client153/Angelica2/Output/3rdsdk/include src/client153/Angelica2/Output/a3dsdk/include
      src/client153/Angelica2/Output/acsdk/include src/client153/Angelica2/Output/afsdk/include
      src/client153/Angelica2/Output/amsdk/include)
    target_link_libraries(client153_aui_xml_test PRIVATE client153_file)
    target_compile_definitions(client153_aui_xml_test PRIVATE WIN32 NDEBUG _LIB UNICODE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(client153_aui_xml_test PRIVATE -fms-extensions -fexceptions
      "SHELL:-include ${PROJECT_SOURCE_DIR}/src/client153/platform/common/Windows.h"
      "SHELL:-include ${PROJECT_SOURCE_DIR}/src/client153/platform/common/sdk_forward.h"
      "SHELL:-include ${PROJECT_SOURCE_DIR}/src/client153/Angelica2/AngelicaCommon/Header/AChar.h"
      "SHELL:-include ${PROJECT_SOURCE_DIR}/src/client153/Angelica2/AngelicaCommon/Header/ATime.h")
    foreach(widget AUIListBox AUIComboBox AUIEditBox AUILabel AUIStillImageButton AUICheckBox AUIImagePicture AUIDialog)
      target_compile_options(client153_aui_xml_test PRIVATE "SHELL:-include ${PROJECT_SOURCE_DIR}/src/client153/Common/AUInterface2/${widget}.h")
    endforeach()
    target_link_options(client153_aui_xml_test PRIVATE -fexceptions "-sENVIRONMENT=node")
    add_test(NAME client153_aui_xml COMMAND client153_aui_xml_test)
  endif()
  add_executable(client153_winui_types_test tests/client153/winui_types.cpp)
  target_link_libraries(client153_winui_types_test PRIVATE client153_common)
  add_test(NAME client153_winui_types COMMAND client153_winui_types_test)
  add_executable(client153_aarray_const_test tests/client153/aarray_const.cpp)
  target_link_libraries(client153_aarray_const_test PRIVATE client153_common)
  add_test(NAME client153_aarray_const COMMAND client153_aarray_const_test)
  add_executable(client153_exception_decl_test tests/client153/exception_decl.cpp)
  target_link_libraries(client153_exception_decl_test PRIVATE client153_common)
  add_test(NAME client153_exception_decl COMMAND client153_exception_decl_test)
  add_executable(client153_findfiles_test tests/client153/findfiles_wasm.cpp)
  target_link_libraries(client153_findfiles_test PRIVATE client153_platform)
  add_test(NAME client153_findfiles COMMAND client153_findfiles_test)
  add_executable(client153_wincrt_test tests/client153/wincrt.cpp)
  target_include_directories(client153_wincrt_test PRIVATE src/client153/Angelica2/Output/a3dsdk/include)
  target_link_libraries(client153_wincrt_test PRIVATE client153_common)
  add_test(NAME client153_wincrt COMMAND client153_wincrt_test)
  # U04-B: the wide-char scan formats the audited game call sites use,
  # driven through the shipped parser and all three aliases (swscanf,
  # a_sscanf, wide_sscanf) with MSVC matching-failure semantics.
  add_executable(client153_wide_sscanf_formats_test tests/client153/wide_sscanf_formats.cpp)
  target_include_directories(client153_wide_sscanf_formats_test PRIVATE
    src/client153/platform/common
    src/client153/Angelica2/AngelicaCommon/Header)
  target_link_libraries(client153_wide_sscanf_formats_test PRIVATE client153_platform)
  add_test(NAME client153_wide_sscanf_formats COMMAND client153_wide_sscanf_formats_test)
  # B05: the wide file reads the serverlist parser uses — 16-bit unit
  # readers over a UTF-16 LE fixture shaped like the shipped serverlist.
  add_executable(client153_wide_file_reads_test tests/client153/wide_file_reads.cpp)
  target_include_directories(client153_wide_file_reads_test PRIVATE
    src/client153/platform/common
    src/client153/Angelica2/AngelicaCommon/Header)
  target_link_libraries(client153_wide_file_reads_test PRIVATE client153_platform)
  add_test(NAME client153_wide_file_reads COMMAND client153_wide_file_reads_test)
  # B19: the engine log collapses consecutive identical messages (the
  # first occurrence verbatim, repeats counted) so per-frame draw
  # failures cannot grow the log unbounded.
  add_executable(client153_alog_dedupe_test tests/client153/alog_dedupe.cpp)
  target_link_libraries(client153_alog_dedupe_test PRIVATE client153_common)
  add_test(NAME client153_alog_dedupe COMMAND client153_alog_dedupe_test)
  add_executable(client153_d3d8_caps_test tests/client153/d3d8_caps.cpp)
  target_link_libraries(client153_d3d8_caps_test PRIVATE client153_math)
  add_test(NAME client153_d3d8_caps COMMAND client153_d3d8_caps_test)
  add_executable(client153_win_sal_scope_test tests/client153/win_sal_scope.cpp)
  target_link_libraries(client153_win_sal_scope_test PRIVATE client153_math)
  add_test(NAME client153_win_sal_scope COMMAND client153_win_sal_scope_test)
  add_executable(client153_vsdeclarator_test tests/client153/vsdeclarator.cpp)
  target_link_libraries(client153_vsdeclarator_test PRIVATE client153_math)
  add_test(NAME client153_vsdeclarator COMMAND client153_vsdeclarator_test)
  add_executable(client153_observer_test tests/client153/observer.cpp)
  target_link_libraries(client153_observer_test PRIVATE client153_common)
  target_include_directories(client153_observer_test PRIVATE src/client153/Element/CElementClient)
  add_test(NAME client153_observer COMMAND client153_observer_test)
  add_executable(client153_gnet_marshal_test tests/client153/gnet_marshal.cpp)
  target_include_directories(client153_gnet_marshal_test PRIVATE src/client153/Element/CElementClient/Network/IOLib)
  add_test(NAME client153_gnet_marshal COMMAND client153_gnet_marshal_test)
  add_executable(client153_typelist_test tests/client153/typelist.cpp)
  target_link_libraries(client153_typelist_test PRIVATE client153_common)
  target_include_directories(client153_typelist_test PRIVATE src/client153/Element/CElementClient)
  add_test(NAME client153_typelist COMMAND client153_typelist_test)
  if(NOT EMSCRIPTEN)
    find_package(Threads REQUIRED)
    add_executable(client153_gnet_stream_test tests/client153/gnet_stream.cpp)
    target_link_libraries(client153_gnet_stream_test PRIVATE client153_common Threads::Threads)
    target_include_directories(client153_gnet_stream_test PRIVATE src/client153/Element/CElementClient/Network/IOLib)
    target_compile_definitions(client153_gnet_stream_test PRIVATE __cdecl=)
    add_test(NAME client153_gnet_stream COMMAND client153_gnet_stream_test)
    add_executable(client153_gnet_poll_test tests/client153/gnet_poll.cpp)
    target_link_libraries(client153_gnet_poll_test PRIVATE client153_common Threads::Threads)
    target_include_directories(client153_gnet_poll_test PRIVATE src/client153/Element/CElementClient/Network/IOLib)
    target_compile_definitions(client153_gnet_poll_test PRIVATE __cdecl=)
    add_test(NAME client153_gnet_poll COMMAND client153_gnet_poll_test)
    add_executable(client153_gnet_conf_test tests/client153/gnet_conf.cpp)
    target_link_libraries(client153_gnet_conf_test PRIVATE client153_common Threads::Threads)
    target_include_directories(client153_gnet_conf_test PRIVATE src/client153/Element/CElementClient/Network/IOLib)
    target_compile_definitions(client153_gnet_conf_test PRIVATE __cdecl=)
    add_test(NAME client153_gnet_conf COMMAND client153_gnet_conf_test)
    add_executable(client153_gnet_thread_test tests/client153/gnet_thread.cpp)
    target_link_libraries(client153_gnet_thread_test PRIVATE client153_common Threads::Threads)
    target_include_directories(client153_gnet_thread_test PRIVATE src/client153/Element/CElementClient/Network/IOLib)
    target_compile_definitions(client153_gnet_thread_test PRIVATE __cdecl=)
    add_test(NAME client153_gnet_thread COMMAND client153_gnet_thread_test)
    set_tests_properties(client153_gnet_thread PROPERTIES TIMEOUT 10)
    add_executable(client153_winsync_semantics_test tests/client153/winsync_semantics.cpp)
    target_link_libraries(client153_winsync_semantics_test PRIVATE client153_platform Threads::Threads)
    add_test(NAME client153_winsync_semantics COMMAND client153_winsync_semantics_test)
    set_tests_properties(client153_winsync_semantics PROPERTIES TIMEOUT 30)
    add_executable(client153_winsock_test tests/client153/winsock.cpp)
    target_link_libraries(client153_winsock_test PRIVATE client153_common)
    add_test(NAME client153_winsock COMMAND client153_winsock_test)
  endif()
  add_executable(client153_utf16_test tests/client153/utf16.cpp)
  target_link_libraries(client153_utf16_test PRIVATE client153_platform)
  add_test(NAME client153_utf16 COMMAND client153_utf16_test)
  add_executable(client153_unicode_width_test tests/client153/unicode_width.cpp)
  target_compile_definitions(client153_unicode_width_test PRIVATE UNICODE)
  target_link_libraries(client153_unicode_width_test PRIVATE client153_common)
  add_test(NAME client153_unicode_width COMMAND client153_unicode_width_test)
  add_executable(client153_common_test tests/client153/common.cpp)
  target_link_libraries(client153_common_test PRIVATE client153_common client153_math)
  add_test(NAME client153_common COMMAND client153_common_test)
  add_executable(client153_gb18030_encoder_test tests/client153/gb18030_encoder.cpp)
  target_link_libraries(client153_gb18030_encoder_test PRIVATE client153_common)
  add_test(NAME client153_gb18030_encoder COMMAND client153_gb18030_encoder_test)
  # U04 regression test: drives the REAL shipped parser TU (AWScriptFile.cpp
  # compiled into this target with the engine build's compile environment)
  # over real UTF-16 script fixtures. Self-contained on purpose (PW23-048):
  # client153_engine_original exists only under Emscripten, so linking
  # $<TARGET_OBJECTS:...> broke the native CMake generation.
  add_executable(client153_u04_instance_parse_test
    tests/client153/u04_instance_parse.cpp
    src/client153/Angelica2/AngelicaFile/Source/AWScriptFile.cpp)
  target_compile_definitions(client153_u04_instance_parse_test PRIVATE
    U04_EC_INSTANCE_SOURCE="${CMAKE_SOURCE_DIR}/src/client153/Element/CElementClient/EC_Instance.cpp"
    WIN32 NDEBUG _LIB UNICODE _CRT_SECURE_NO_WARNINGS)
  target_link_libraries(client153_u04_instance_parse_test PRIVATE client153_file client153_common)
  target_include_directories(client153_u04_instance_parse_test PRIVATE
    src/client153/Angelica2/AngelicaFile/Header
    src/client153/Angelica2/AngelicaCommon/Header
    src/client153/platform/common)
  # The compile environment the engine build gives this TU. No
  # -fms-extensions: the native CI compiler is GCC, which rejects it.
  target_compile_options(client153_u04_instance_parse_test PRIVATE -fexceptions
    "SHELL:-include ${CMAKE_SOURCE_DIR}/src/client153/platform/common/Windows.h"
    "SHELL:-include ${CMAKE_SOURCE_DIR}/src/client153/platform/common/sdk_forward.h"
    "SHELL:-include ${CMAKE_SOURCE_DIR}/src/client153/Angelica2/AngelicaCommon/Header/AChar.h"
    "SHELL:-include ${CMAKE_SOURCE_DIR}/src/client153/Angelica2/AngelicaCommon/Header/ATime.h")
  if(EMSCRIPTEN)
    # The AWScriptFile parser and the ALog sink need stack headroom (the
    # default 64 KB overflows in the ALog directory setup).
    target_link_options(client153_u04_instance_parse_test PRIVATE -sSTACK_SIZE=1048576)
  endif()
  # Structural gate (U04-A): the EC_Instance rejection is restored — the
  # diagnostic continue bypass must not return. This runs at every
  # configure (including CI) and fails the build when the bypass returns.
  file(READ ${CMAKE_SOURCE_DIR}/src/client153/Element/CElementClient/EC_Instance.cpp u04_ec_instance_source)
  if(u04_ec_instance_source MATCHES "skipping texture with")
    message(FATAL_ERROR "U04-A regression: the EC_Instance diagnostic bypass (skipping texture) is back in the source — the original return false rejection must stay")
  endif()
  # Structural gate (U05 / PW23-040): the elements.data load failure aborts
  # init again — the parse is fixed (32-bit serialized timestamps) and
  # regression-tested; the boot-diagnostic continuation must not return.
  file(READ ${CMAKE_SOURCE_DIR}/src/client153/Element/CElementClient/EC_Game.cpp u05_ec_game_source)
  if(u05_ec_game_source MATCHES "elements.data load failed \\(continuing")
    message(FATAL_ERROR "U05 regression: the EC_Game elements.data diagnostic bypass is back in the source — the original return false rejection must stay")
  endif()
  # Structural gate (U05 / B07): the login scene and terrain loads abort
  # again — both load in the browser boot (case-folded VFS lookup), so the
  # diagnostic continuations must not return.
  file(READ ${CMAKE_SOURCE_DIR}/src/client153/Element/CElementClient/EC_World.cpp u05_ec_world_source)
  if(u05_ec_world_source MATCHES "continuing for login diagnostic|continuing without terrain")
    message(FATAL_ERROR "U05 regression: the EC_World scene/terrain diagnostic bypasses are back in the source — the original return false rejections must stay")
  endif()
  configure_file(${CMAKE_SOURCE_DIR}/src/client153/Element/CElementClient/EC_Instance.cpp
                 ${CMAKE_CURRENT_BINARY_DIR}/EC_Instance_source.cpp COPYONLY)
  add_test(NAME client153_u04_instance_parse COMMAND client153_u04_instance_parse_test)
  # PW23-040 / U05: the elements.data binary parse contract, driven through
  # the real shipped parser TU (elementdataman, compiled with the game
  # build's TU environment — the port's Windows.h provides the fixed-width
  # types, so the TU compiles under native GCC and Emscripten alike). The
  # full byte-exact load runs when the authorized asset path is passed as
  # the executable's argument (the native LP64 and WASM32 baselines the
  # PW23-040 review requires); the ctest run (CI) exercises the crafted
  # negative fixtures.
  add_executable(client153_elements_data_parse_test
    tests/client153/elements_data_parse.cpp
    src/client153/Element/CCommon/elementdataman.cpp)
  target_compile_definitions(client153_elements_data_parse_test PRIVATE
    WIN32 NDEBUG _WINDOWS UNICODE _ELEMENTCLIENT _USE_32BIT_TIME_T
    ENABLE_SKILL RESOURCE_BATTLE _WIN32_DCOM _LIB _ANGELICA2 _ANGELICA_AUDIO _FW_EDITOR)
  target_include_directories(client153_elements_data_parse_test PRIVATE
    src/client153/Element/CCommon
    src/client153/Angelica2/AngelicaCommon/Header
    src/client153/platform/common)
  target_compile_options(client153_elements_data_parse_test PRIVATE -fexceptions -w
    "SHELL:-include ${CMAKE_SOURCE_DIR}/src/client153/platform/common/Windows.h"
    "SHELL:-include ${CMAKE_SOURCE_DIR}/src/client153/platform/common/sdk_forward.h"
    "SHELL:-include ${CMAKE_SOURCE_DIR}/src/client153/Angelica2/AngelicaCommon/Header/AChar.h"
    "SHELL:-include ${CMAKE_SOURCE_DIR}/src/client153/Angelica2/AngelicaCommon/Header/ATime.h")
  target_link_libraries(client153_elements_data_parse_test PRIVATE client153_common client153_platform)
  if(EMSCRIPTEN)
    target_link_options(client153_elements_data_parse_test PRIVATE
      -sSTACK_SIZE=1048576 -sALLOW_MEMORY_GROWTH=1 -sMAXIMUM_MEMORY=4GB
      -sNODERAWFS=1 -sENVIRONMENT=node)
  endif()
  add_test(NAME client153_elements_data_parse COMMAND client153_elements_data_parse_test)
  # B20: the case-folded VFS lookup preserves the caller's semantics —
  # driven against the real Emscripten MEMFS (mismatched-case reads,
  # creation under a folded parent, missing leaves, no-follow lstat,
  # ".." and nested paths). Emscripten-only: the hook patches the
  # browser MEMFS; the native seam installer is a no-op.
  if(EMSCRIPTEN)
  add_executable(client153_vfs_casefold_fs_test tests/client153/vfs_casefold_fs.cpp)
  target_include_directories(client153_vfs_casefold_fs_test PRIVATE
    src/client153/platform/web src/client153/platform/common)
  target_link_libraries(client153_vfs_casefold_fs_test PRIVATE client153_webgl_caps)
  target_link_options(client153_vfs_casefold_fs_test PRIVATE
    -sSTACK_SIZE=1048576 -sENVIRONMENT=node)
  add_test(NAME client153_vfs_casefold_fs COMMAND client153_vfs_casefold_fs_test)
  endif()
  # B02/B10: the game's real package flow in the WASM target — the split
  # models package opens and the game's exact GB18030 skin spelling
  # resolves through the shipped AFilePackMan/AFilePackGame code. The
  # full checks run when the authorized asset dir is passed as the
  # executable's argument; the ctest run (CI) reports the skip.
  if(EMSCRIPTEN)
  add_executable(client153_pack_lookup_test tests/client153/pack_lookup.cpp)
  target_include_directories(client153_pack_lookup_test PRIVATE
    src/client153/platform/common src/client153/Angelica2/AngelicaCommon/Header
    src/client153/Angelica2/AngelicaFile/Header)
  target_link_libraries(client153_pack_lookup_test PRIVATE client153_file client153_common)
  target_link_options(client153_pack_lookup_test PRIVATE
    -sSTACK_SIZE=1048576 -sALLOW_MEMORY_GROWTH=1 -sMAXIMUM_MEMORY=4GB
    -sNODERAWFS=1 -sENVIRONMENT=node)
  add_test(NAME client153_pack_lookup COMMAND client153_pack_lookup_test)
  endif()
  # B08: the vs.1.1 assembler covers the shipped shader corpus — every
  # vertex shader in the authorized shaders.pck assembles into GLSL ES
  # 300 (the corpus drive goes through the independently validated
  # Python reader).
  if(EMSCRIPTEN)
  add_executable(client153_vs_assembler_corpus_test tests/client153/vs_assembler_corpus.cpp)
  target_include_directories(client153_vs_assembler_corpus_test PRIVATE
    src/client153/platform/web src/client153/platform/common)
  target_link_libraries(client153_vs_assembler_corpus_test PRIVATE client153_platform client153_webgl_caps)
  target_link_options(client153_vs_assembler_corpus_test PRIVATE
    -sSTACK_SIZE=1048576 -sENVIRONMENT=node -sNODERAWFS=1)
  add_test(NAME client153_vs_assembler_corpus COMMAND client153_vs_assembler_corpus_test)
  # B08: the ps.1.x assembler covers the shipped pixel-shader corpus —
  # every ps.1.1/ps.1.4 shader assembles into GLSL ES 300 and the five
  # ps_2_0 gfx effects fail with the honest classification.
  add_executable(client153_ps_assembler_corpus_test tests/client153/ps_assembler_corpus.cpp)
  target_include_directories(client153_ps_assembler_corpus_test PRIVATE
    src/client153/platform/web src/client153/platform/common)
  target_link_libraries(client153_ps_assembler_corpus_test PRIVATE client153_platform client153_webgl_caps)
  target_link_options(client153_ps_assembler_corpus_test PRIVATE
    -sSTACK_SIZE=1048576 -sENVIRONMENT=node -sNODERAWFS=1)
  add_test(NAME client153_ps_assembler_corpus COMMAND client153_ps_assembler_corpus_test)
  endif()
  add_executable(client153_tracks_test tests/client153/tracks.cpp)
  target_link_libraries(client153_tracks_test PRIVATE client153_animation)
  add_test(NAME client153_tracks COMMAND client153_tracks_test)
  add_executable(client153_math_test tests/client153/math.cpp)
  target_link_libraries(client153_math_test PRIVATE client153_math)
  add_test(NAME client153_math COMMAND client153_math_test)
  add_executable(client153_file_test tests/client153/file.cpp)
  target_link_libraries(client153_file_test PRIVATE client153_file client153_formats)
  add_test(NAME client153_file COMMAND client153_file_test)
  add_executable(client153_time_test tests/client153/time.cpp)
  target_link_libraries(client153_time_test PRIVATE client153_common)
  add_test(NAME client153_time COMMAND client153_time_test)
  add_executable(client153_astring_test tests/client153/astring.cpp)
  target_link_libraries(client153_astring_test PRIVATE client153_common)
  add_test(NAME client153_astring COMMAND client153_astring_test)
  add_executable(client153_skinning_test tests/client153/skinning.cpp)
  target_link_libraries(client153_skinning_test PRIVATE client153_skinning)
  add_test(NAME client153_skinning COMMAND client153_skinning_test)
  add_executable(client153_ski_test tests/client153/ski_loader.cpp)
  target_link_libraries(client153_ski_test PRIVATE client153_formats)
  add_test(NAME client153_ski COMMAND client153_ski_test)
  add_executable(client153_mox_test tests/client153/mox_loader.cpp)
  target_link_libraries(client153_mox_test PRIVATE client153_formats)
  add_test(NAME client153_mox COMMAND client153_mox_test)
  add_executable(client153_dds_test tests/client153/dds_loader.cpp)
  target_link_libraries(client153_dds_test PRIVATE client153_formats)
  add_test(NAME client153_dds COMMAND client153_dds_test)
  add_executable(client153_skeleton_test tests/client153/skeleton.cpp)
  target_link_libraries(client153_skeleton_test PRIVATE client153_skeleton client153_math)
  add_test(NAME client153_skeleton COMMAND client153_skeleton_test)
  add_executable(client153_file_family_test tests/client153/file_family.cpp)
  target_link_libraries(client153_file_family_test PRIVATE client153_file)
  add_test(NAME client153_file_family COMMAND client153_file_family_test)
  add_executable(client153_awstring_test tests/client153/awstring.cpp)
  target_link_libraries(client153_awstring_test PRIVATE client153_common)
  add_test(NAME client153_awstring COMMAND client153_awstring_test)
  add_executable(client153_skeleton_closure_test tests/client153/skeleton_closure.cpp)
  target_link_libraries(client153_skeleton_closure_test PRIVATE client153_skeleton client153_file client153_skinning)
  set(PW153_TEST_BON_PATH "" CACHE FILEPATH "Optional authorized wolf BON for skeleton integration test")
  set(PW153_TEST_SKI_PATH "" CACHE FILEPATH "Optional authorized wolf SKI for skinning integration test")
  if(PW153_TEST_SKI_PATH AND NOT PW153_TEST_BON_PATH)
    message(FATAL_ERROR "PW153_TEST_SKI_PATH requires PW153_TEST_BON_PATH")
  endif()
  if(PW153_TEST_SKI_PATH AND (NOT EXISTS "${PW153_TEST_SKI_PATH}" OR IS_DIRECTORY "${PW153_TEST_SKI_PATH}"))
    message(FATAL_ERROR "PW153_TEST_SKI_PATH must be an existing SKI file")
  endif()
  if(PW153_TEST_BON_PATH)
    if(NOT EXISTS "${PW153_TEST_BON_PATH}" OR IS_DIRECTORY "${PW153_TEST_BON_PATH}")
      message(FATAL_ERROR "PW153_TEST_BON_PATH must be an existing BON file")
    endif()
    if(EMSCRIPTEN)
      target_link_options(client153_skeleton_closure_test PRIVATE
        "SHELL:--preload-file \"${PW153_TEST_BON_PATH}@/pw153-test-wolf.bon\"")
      set_property(TARGET client153_skeleton_closure_test APPEND PROPERTY LINK_DEPENDS "${PW153_TEST_BON_PATH}")
      set(PW153_SKIN_TEST_ARGS /pw153-test-wolf.bon)
      if(PW153_TEST_SKI_PATH)
        target_link_options(client153_skeleton_closure_test PRIVATE "SHELL:--preload-file \"${PW153_TEST_SKI_PATH}@/pw153-test-wolf.ski\"")
        set_property(TARGET client153_skeleton_closure_test APPEND PROPERTY LINK_DEPENDS "${PW153_TEST_SKI_PATH}")
        list(APPEND PW153_SKIN_TEST_ARGS /pw153-test-wolf.ski)
      endif()
      add_test(NAME client153_skeleton_closure COMMAND client153_skeleton_closure_test ${PW153_SKIN_TEST_ARGS})
    else()
      set(PW153_SKIN_TEST_ARGS "${PW153_TEST_BON_PATH}")
      if(PW153_TEST_SKI_PATH)
        list(APPEND PW153_SKIN_TEST_ARGS "${PW153_TEST_SKI_PATH}")
      endif()
      add_test(NAME client153_skeleton_closure COMMAND client153_skeleton_closure_test ${PW153_SKIN_TEST_ARGS})
    endif()
  else()
    add_test(NAME client153_skeleton_closure COMMAND client153_skeleton_closure_test)
  endif()
endif()
