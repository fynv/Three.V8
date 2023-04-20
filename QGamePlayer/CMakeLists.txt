cmake_minimum_required (VERSION 3.0)

project(QGamePlayer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

SET(QT_ROOT ${QT_ROOT} CACHE PATH "Qt path" FORCE)
SET(Qt6_DIR ${QT_ROOT}/lib/cmake/Qt6)

find_package(Qt6 COMPONENTS Gui REQUIRED)
find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS OpenGLWidgets REQUIRED)


set(SOURCES
main.cpp
QGamePlayer.ui
QGamePlayer.cpp
GLControl.cpp
PlayerWindow.ui
PlayerWindow.cpp
resources/resources.qrc
)

set(HEADERS 
QGamePlayer.h
PlayerWindow.h
GLControl.h
)

set (INCLUDE_DIR
.
../ThreeEngine
../ThreeMM
../Three.V8
../thirdparty/glew/include
../thirdparty/glm/glm
../packages/v8-v143-x64.10.0.139.9/include
)

set (LINK_DIR
../x64/Release
../thirdparty/draco/lib
../thirdparty/glew/lib
../thirdparty/portaudio/lib
../thirdparty/webp/lib
../packages/zeroc.openssl.v143.1.1.1.2/build/native/lib/x64/Release
../packages/FFmpeg.Nightly.20200831.1.0/build/native/lib/x64
../packages/v8-v143-x64.10.0.139.9/lib/Release
)

set (LINK_LIBS 
Qt6::Widgets
Qt6::OpenGLWidgets
ThreeEngine.lib
ThreeMM.lib
Three.V8.lib
draco.lib
opengl32.lib
glew.lib
portaudio.lib
libsharpyuv.lib
libwebp.lib
libwebpdecoder.lib
libwebpdemux.lib
libwebpmux.lib
libcrypto.lib
libssl.lib
avcodec.lib
avdevice.lib
avfilter.lib
avformat.lib
avutil.lib
postproc.lib
swresample.lib
swscale.lib
v8.dll.lib
v8_libbase.dll.lib
v8_libplatform.dll.lib
zlib.dll.lib
)

set (DLLS_TO_COPY
${QT_ROOT}/bin/Qt6Core.dll
${QT_ROOT}/bin/Qt6Gui.dll
${QT_ROOT}/bin/Qt6Widgets.dll
${QT_ROOT}/bin/Qt6OpenGL.dll
${QT_ROOT}/bin/Qt6OpenGLWidgets.dll
../packages/FFmpeg.Nightly.20200831.1.0/build/native/bin/x64/avcodec-58.dll
../packages/FFmpeg.Nightly.20200831.1.0/build/native/bin/x64/avdevice-58.dll
../packages/FFmpeg.Nightly.20200831.1.0/build/native/bin/x64/avfilter-7.dll
../packages/FFmpeg.Nightly.20200831.1.0/build/native/bin/x64/avformat-58.dll
../packages/FFmpeg.Nightly.20200831.1.0/build/native/bin/x64/avutil-56.dll
../packages/FFmpeg.Nightly.20200831.1.0/build/native/bin/x64/postproc-55.dll
../packages/FFmpeg.Nightly.20200831.1.0/build/native/bin/x64/swresample-3.dll
../packages/FFmpeg.Nightly.20200831.1.0/build/native/bin/x64/swscale-5.dll
../packages/v8.redist-v143-x64.10.0.139.9/lib/Release/icudtl.dat
../packages/v8.redist-v143-x64.10.0.139.9/lib/Release/icui18n.dll
../packages/v8.redist-v143-x64.10.0.139.9/lib/Release/icuuc.dll
../packages/v8.redist-v143-x64.10.0.139.9/lib/Release/v8.dll
../packages/v8.redist-v143-x64.10.0.139.9/lib/Release/v8_libbase.dll
../packages/v8.redist-v143-x64.10.0.139.9/lib/Release/v8_libplatform.dll
../packages/v8.redist-v143-x64.10.0.139.9/lib/Release/zlib.dll
)


set (DEFINES  ${DEFINES}
-D"_CRT_SECURE_NO_DEPRECATE"  
-D"_SCL_SECURE_NO_DEPRECATE"
-D"GLEW_STATIC"
)


include_directories(${INCLUDE_DIR})
add_definitions(${DEFINES})
add_executable (QGamePlayer ${SOURCES} ${HEADERS})
set_property(TARGET QGamePlayer PROPERTY WIN32_EXECUTABLE true)

target_link_directories(QGamePlayer PUBLIC ${LINK_DIR})
target_link_libraries(QGamePlayer ${LINK_LIBS})

install(FILES ${QT_ROOT}/plugins/platforms/qwindows.dll DESTINATION ./platforms)
install(FILES ${DLLS_TO_COPY} DESTINATION .)
install(TARGETS QGamePlayer DESTINATION .)


