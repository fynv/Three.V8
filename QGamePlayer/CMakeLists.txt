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
SET(Qt6Core_DIR ${QT_ROOT}/lib/cmake/Qt6Core)
SET(Qt6CoreTools_DIR ${QT_ROOT}/lib/cmake/Qt6CoreTools)
SET(Qt6Gui_DIR ${QT_ROOT}/lib/cmake/Qt6Gui)
SET(Qt6GuiTools_DIR ${QT_ROOT}/lib/cmake/Qt6GuiTools)
SET(Qt6Widgets_DIR ${QT_ROOT}/lib/cmake/Qt6Widgets)
SET(Qt6WidgetsTools_DIR ${QT_ROOT}/lib/cmake/Qt6WidgetsTools)


find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS OpenGLWidgets REQUIRED)


set(SOURCES
main.cpp
QGamePlayer.ui
QGamePlayer.cpp
GLControl.cpp
PlayerWindow.ui
PlayerWindow.cpp
WebView2Widget.cpp
WebPlayerWindow.ui
WebPlayerWindow.cpp
resources/resources.qrc
)

set(HEADERS 
QGamePlayer.h
GLControl.h
PlayerWindow.h
WebView2Widget.h
WebPlayerWindow.h
)

set (INCLUDE_DIR
.
../ThreeEngine
../ThreeMM
../Three.V8
../thirdparty/glew/include
../thirdparty/glm/glm
../thirdparty/webview2/include
../packages/v8-v143-x64.11.9.169.4/include
)

set (LINK_DIR
../x64/Release
../thirdparty/draco/lib
../thirdparty/glew/lib
../thirdparty/portaudio/lib
../thirdparty/webp/lib
../thirdparty/webview2/lib
../packages/boost_url-vc143.1.84.0/lib/native
../packages/zeroc.openssl.v143.1.1.1.3/build/native/lib/x64/Release
../packages/FFmpeg.Nightly.20200831.1.0/build/native/lib/x64
../packages/v8-v143-x64.11.9.169.4/lib/Release
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
WebView2LoaderStatic.lib
version.lib
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
third_party_abseil-cpp_absl.dll.lib
third_party_icu_icui18n.dll.lib
third_party_zlib.dll.lib
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
../packages/v8.redist-v143-x64.11.9.169.4/lib/Release/icudtl.dat
../packages/v8.redist-v143-x64.11.9.169.4/lib/Release/icuuc.dll
../packages/v8.redist-v143-x64.11.9.169.4/lib/Release/v8.dll
../packages/v8.redist-v143-x64.11.9.169.4/lib/Release/v8_libbase.dll
../packages/v8.redist-v143-x64.11.9.169.4/lib/Release/v8_libplatform.dll
../packages/v8.redist-v143-x64.11.9.169.4/lib/Release/third_party_abseil-cpp_absl.dll
../packages/v8.redist-v143-x64.11.9.169.4/lib/Release/third_party_icu_icui18n.dll
../packages/v8.redist-v143-x64.11.9.169.4/lib/Release/third_party_zlib.dll
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

SET(CMAKE_INSTALL_PREFIX  ../QBin CACHE PATH "Install path" FORCE)

install(FILES ${QT_ROOT}/plugins/platforms/qwindows.dll DESTINATION ./platforms)
install(FILES ${DLLS_TO_COPY} DESTINATION .)
install(TARGETS QGamePlayer DESTINATION .)


