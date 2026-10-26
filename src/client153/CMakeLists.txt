# Original Common interface, with portable string storage/formatting corrections.
#	Emscripten ships zlib in its sysroot (link -lz directly); native builds
#	use the system zlib through CMake's FindZLIB.
if(EMSCRIPTEN)
  #	Emscripten's zlib port flags live in the top-level CMakeLists so test
  #	targets outside this directory scope link the port too.
  set(PW153_ZLIB_LINK "")
else()
  find_package(ZLIB REQUIRED)
  set(PW153_ZLIB_LINK ZLIB::ZLIB)
endif()

add_library(client153_platform STATIC platform/common/clock.cpp platform/common/format.cpp platform/common/utf16.cpp platform/common/widestr.cpp)
target_include_directories(client153_platform PUBLIC platform/common)
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
  add_executable(client153 platform/web/runtime.cpp)
  target_link_libraries(client153 PRIVATE client153_formats client153_file client153_math)
  target_compile_options(client153 PRIVATE -Wall -Wextra -Werror)
  set_target_properties(client153 PROPERTIES CXX_EXTENSIONS ON)
  target_link_options(client153 PRIVATE
    --no-entry
    "-sMIN_WEBGL_VERSION=2" "-sMAX_WEBGL_VERSION=2"
    "-sMODULARIZE=1" "-sEXPORT_ES6=1" "-sENVIRONMENT=web"
    "-sFILESYSTEM=1" "-sALLOW_MEMORY_GROWTH=1"
    "-sEXPORTED_FUNCTIONS=[_malloc,_free,_client_init,_client_load,_client_load_file,_client_texture,_client_texture_name,_client_texture_name_size,_client_mesh_count,_client_draw,_client_error,_client_dispose]"
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
