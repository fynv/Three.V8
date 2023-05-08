cmake_minimum_required (VERSION 3.0)

project(QGameDev)

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
GLControl.cpp
JsonUtils.cpp
MainWindow.ui
MainWindow.cpp
WebView2Widget.cpp
JsonData.cpp
DlgNewFile.ui
DlgNewFile.cpp
DlgNewDir.ui
DlgNewDir.cpp
DlgEditTarget.ui
DlgEditTarget.cpp
DlgProjectSettings.ui
DlgProjectSettings.cpp
Editor.cpp
HelpPage.ui
HelpPage.cpp
JSEditor.ui
JSEditor.cpp
JsonEditor.ui
JsonEditor.cpp
XMLEditor.ui
XMLEditor.cpp
ColorTuner.ui
ColorTuner.cpp
Tuner.cpp
FogTuner.ui
FogTuner.cpp
SkyTuner.ui
SkyTuner.cpp
UniformSkyTuner.ui
UniformSkyTuner.cpp
HemisphereSkyTuner.ui
HemisphereSkyTuner.cpp
CubeSkyTuner.ui
CubeSkyTuner.cpp
BackgroundSceneTuner.ui
BackgroundSceneTuner.cpp
resources/resources.qrc
)

set(HEADERS
ColorUtils.h
GLControl.h
JsonUtils.h
MainWindow.h
WebView2Widget.h
JsonData.h
DlgNewFile.h
DlgNewDir.h
DlgEditTarget.h
DlgProjectSettings.h
Editor.h
HelpPage.h
JSEditor.h
JsonEditor.h
XMLEditor.h
ColorTuner.h
Tuner.h
FogTuner.h
SkyTuner.h
UniformSkyTuner.h
HemisphereSkyTuner.h
CubeSkyTuner.h
BackgroundSceneTuner.h
)

set (INCLUDE_DIR
.
../ThreeEngine
../ThreeMM
../Three.V8
../thirdparty/glew/include
../thirdparty/glm/glm
../thirdparty/webview2/include
../packages/v8-v143-x64.10.0.139.9/include
)

set (LINK_DIR
../x64/Release
../thirdparty/draco/lib
../thirdparty/glew/lib
../thirdparty/portaudio/lib
../thirdparty/webp/lib
../thirdparty/webview2/lib
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
zlib.dll.lib
)

set (DLLS_TO_COPY

)


set (DEFINES  ${DEFINES}
-D"_CRT_SECURE_NO_DEPRECATE"  
-D"_SCL_SECURE_NO_DEPRECATE"
-D"GLEW_STATIC"
)


include_directories(${INCLUDE_DIR})
add_definitions(${DEFINES})
add_executable (QGameDev ${SOURCES} ${HEADERS})
set_property(TARGET QGameDev PROPERTY WIN32_EXECUTABLE false)

target_link_directories(QGameDev PUBLIC ${LINK_DIR})
target_link_libraries(QGameDev ${LINK_LIBS})

install(FILES ${DLLS_TO_COPY} DESTINATION .)
install(TARGETS QGameDev DESTINATION .)


