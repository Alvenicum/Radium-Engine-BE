cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0042 NEW)

project(HeadlessExample VERSION 1.0.2)

# ------------------------------------------------------------------------------
# set wanted application defaults for cmake settings
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
# Set default install location to installed-<Compiler_ID> folder in build dir we do not want to
# install to /usr by default
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX
        "${CMAKE_CURRENT_BINARY_DIR}/installed-${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}"
        CACHE PATH "Install path prefix, prepended onto install directories." FORCE
    )
    message("Set install prefix to ${CMAKE_INSTALL_PREFIX}")
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# ------------------------------------------------------------------------------
find_package(Radium REQUIRED COMPONENTS Headless)

option(USE_RADIUM_IO_ASSIMPLOADER "Use assimp data loader from Radium IO" ON)
message(STATUS "Configuring ${PROJECT_NAME} application with Radium ${Radium_VERSION}")

set(app_sources main.cpp)

add_executable(${PROJECT_NAME} ${app_sources})

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE /MP
                /W4
                /wd4251
                /wd4592
                /wd4127
                /Zm200
                $<$<CONFIG:Release>:
                /Gw
                /GS-
                /GL
                /GF
                >
        PUBLIC
    )
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC Radium::Headless)

get_target_property(USE_ASSIMP Radium::IO IO_HAS_ASSIMP)
if(${USE_ASSIMP})
    target_compile_definitions(${PROJECT_NAME} PRIVATE "IO_HAS_ASSIMP")
endif()

target_compile_options(${PROJECT_NAME} PRIVATE PUBLIC ${DEFAULT_COMPILE_OPTIONS} INTERFACE)

# configure the application
configure_radium_app(
    NAME ${PROJECT_NAME} PREFIX Examples/Headless RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Assets
)
