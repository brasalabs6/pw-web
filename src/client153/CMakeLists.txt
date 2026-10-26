# Original Common interface, with portable string storage/formatting corrections.
add_library(client153_platform STATIC platform/common/clock.cpp)
target_include_directories(client153_platform PUBLIC platform/common)
target_compile_features(client153_platform PUBLIC cxx_std_17)
target_compile_options(client153_platform PRIVATE -Wall -Wextra -Werror -pedantic)
add_library(client153_common STATIC
  Angelica2/AngelicaCommon/Source/AString.cpp
  Angelica2/AngelicaCommon/Source/ATime.cpp)
target_link_libraries(client153_common PRIVATE client153_platform)
target_include_directories(client153_common PUBLIC Angelica2/AngelicaCommon/Header)
target_compile_features(client153_common PUBLIC cxx_std_17)
target_compile_options(client153_common PRIVATE -Wall -Wextra -Werror -pedantic)
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
  Angelica2/Angelica3D/port/dds.cpp)
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

add_library(client153_file STATIC Angelica2/AngelicaFile/Source/AFile.cpp)
target_include_directories(client153_file PUBLIC Angelica2/AngelicaFile/Header)
target_link_libraries(client153_file PUBLIC client153_common)
target_compile_options(client153_file PRIVATE -Wall -Wextra -Werror -pedantic)

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
