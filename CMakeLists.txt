cmake_minimum_required(VERSION 3.31)
project(microphone_detector)

set(CMAKE_CXX_STANDARD 20)

# Encontrar pacotes X11 necessários
find_package(X11 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(XCOMPOSITE REQUIRED xcomposite)
pkg_check_modules(XDAMAGE REQUIRED xdamage)
pkg_check_modules(XRENDER REQUIRED xrender)
pkg_check_modules(XFIXES REQUIRED xfixes)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)

# Incluir diretórios de cabeçalho
include_directories(
        ${X11_INCLUDE_DIR}
        ${XCOMPOSITE_INCLUDE_DIRS}
        ${XDAMAGE_INCLUDE_DIRS}
        ${XRENDER_INCLUDE_DIRS}
        ${XFIXES_INCLUDE_DIRS}
        ${PORTAUDIO_INCLUDE_DIRS}
)

add_executable(microphone_detector microphone_detector.cpp)

target_link_libraries(microphone_detector
        ${PORTAUDIO_LIBRARIES}
        ${X11_LIBRARIES}
        ${XCOMPOSITE_LIBRARIES}
        ${XDAMAGE_LIBRARIES}
        ${XRENDER_LIBRARIES}
        ${XFIXES_LIBRARIES}
)