###############################################################################
# (c) Copyright, Real-Time Innovations 2018-2019
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
################################################################################

cmake_minimum_required(VERSION 3.8)
cmake_policy(SET CMP0048 NEW) # Support VERSION in project
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW) # Support *_ROOT variables
endif()

# Build configuration variables. To overwrite the default value, run cmake with
# -Dvar=value. These two variables must be before the project call since
# toolchains need them.
option(BUILD_SHARED_LIBS "Libraries type; TRUE for shared, FALSE for static" FALSE)
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type: 'Debug' or 'Release'")

set(RTI_CONNEXT_PRO_TYPE "pro")
set(RTI_CONNEXT_MICRO_TYPE "micro")

set(RTI_CONNEXT_TYPE ${RTI_CONNEXT_MICRO_TYPE} CACHE STRING "Connext DDS product type: 'pro' or 'micro'")
set_property(CACHE RTI_CONNEXT_TYPE PROPERTY STRINGS
                 ${RTI_CONNEXT_PRO_TYPE}
                 ${RTI_CONNEXT_MICRO_TYPE})

# Prerequisite environment variables
if (${RTI_CONNEXT_TYPE} MATCHES ${RTI_CONNEXT_PRO_TYPE})
    if (NOT DEFINED ENV{NDDSHOME})
        message(FATAL_ERROR "[RTI] NDDSHOME is not set. Please set to Connext DDS Pro installation directory.")
    endif()
    set(CONNEXTHOME $ENV{NDDSHOME})
    set(NDDSHOME $ENV{NDDSHOME})
    set(RTIDDSGEN_OPTS "")

    if (NOT DEFINED ENV{ROXMLHOME})
        message(FATAL_ERROR "[RTI] ROXMLHOME is not set.")
    endif()
else()
    if (NOT DEFINED ENV{RTIMEHOME})
        message(FATAL_ERROR "[RTI] RTIMEHOME is not set. Please set to Connext DDS Micro installation directory.")
    endif()
    set(CONNEXTHOME $ENV{RTIMEHOME})
    set(RTIMEHOME $ENV{RTIMEHOME})
    set(RTIDDSGEN_OPTS "-micro")
    add_definitions(-DRTI_CONNEXT_MICRO)
endif()

if (NOT DEFINED ENV{RTITSSHOME})
    message(FATAL_ERROR "[RTI] RTITSSHOME is not set. Please set to Connext TSS installation directory.")
endif()

if (NOT DEFINED ENV{RTITSSARCH})
    message(FATAL_ERROR "[RTI] RTITSSARCH is not set. Please set to the name of the platform architecture (e.g. x64Linux3gcc4.8.2).")
endif()

if (NOT DEFINED ENV{JREHOME})
    message(FATAL_ERROR "[RTI] JREHOME is not set. Please set to a JRE 1.6 or 1.7 installation.")
endif()

set(ROXMLHOME $ENV{ROXMLHOME})
set(RTITSSHOME $ENV{RTITSSHOME})
set(RTI_TSS_PLATFORM $ENV{RTITSSARCH})
set(DDSGENHOME ${RTITSSHOME})

# Toolchain (host or cross-compilation) is included if available
set(RTI_TSS_ARCH_TOOLCHAIN
     ${RTITSSHOME}/resource/cmake/Toolchains/Architecture/${RTI_TSS_PLATFORM}.cmake)

include(${RTI_TSS_ARCH_TOOLCHAIN} OPTIONAL RESULT_VARIABLE FOUND_TOOLCHAIN)
if (FOUND_TOOLCHAIN)
    message(STATUS "[RTI] Included toolchain ......: ${FOUND_TOOLCHAIN}")
endif()

# We need to setup the module path before 'project()' loads any file.
list(APPEND CMAKE_MODULE_PATH ${RTITSSHOME}/resource/cmake/Modules)
list(APPEND CMAKE_MODULE_PATH ${RTITSSHOME}/resource/cmake/Toolchains)

project(perftest C CXX)
include(ConnextTss)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wno-deprecated-register)


# Toolchain for this platform
include(Architecture/${RTI_TSS_PLATFORM} OPTIONAL)

message(STATUS "[RTI] RTI_CONNEXT_TYPE ................: ${RTI_CONNEXT_TYPE}")
message(STATUS "[RTI] RTITSSHOME ......................: ${RTITSSHOME}")
message(STATUS "[RTI] CONNEXTHOME .....................: ${CONNEXTHOME}")
message(STATUS "[RTI] CMAKE_BUILD_TYPE ................: ${CMAKE_BUILD_TYPE}")
message(STATUS "[RTI] PLATFORM_ARCH ...................: ${PLATFORM_ARCH}")
message(STATUS "[RTI] RTI_TSS_ENABLE_FACE_COMPLIANCE...: ${RTI_TSS_ENABLE_FACE_COMPLIANCE}")

# static libs only
if(CMAKE_BUILD_TYPE MATCHES "Release")
    set(LIB_SUFFIX "z.a")
else()
    set(LIB_SUFFIX "zd.a")
endif()

# Includes
if (${RTI_CONNEXT_TYPE} MATCHES ${RTI_CONNEXT_PRO_TYPE})
    list(APPEND RTITSS_INC_DIRS  ${RTITSSHOME}/include
                                 ${RTITSSHOME}/include/pro)
    list(APPEND CONNEXT_INC_DIRS ${NDDSHOME}/include
                                 ${NDDSHOME}/include/ndds)
else()
    list(APPEND RTITSS_INC_DIRS  ${RTITSSHOME}/include
                                 ${RTITSSHOME}/include/micro)
    list(APPEND CONNEXT_INC_DIRS ${RTIMEHOME}/include
                                 ${RTIMEHOME}/include/rti_me)
endif()

set(GEN_HEADERS ${GEN_OUTPUT_DIR}/HelloGoodbye.h
                ${GEN_OUTPUT_DIR}/HelloGoodbye.hxx
                ${GEN_OUTPUT_DIR}/HelloGoodbyeSupport.h
                ${GEN_OUTPUT_DIR}/HelloGoodbyePlugin.h
                ${GEN_OUTPUT_DIR}/HelloGoodbye_TSS.hpp)

set(GEN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/perftest.c
                ${CMAKE_CURRENT_SOURCE_DIR}/perftestSupport.c
                ${CMAKE_CURRENT_SOURCE_DIR}/perftestPlugin.c
                ${CMAKE_CURRENT_SOURCE_DIR}/perftest_TypeSupport.c
                ${CMAKE_CURRENT_SOURCE_DIR}/perftest_TypedTS_Impl.cpp)

ADD_CUSTOM_COMMAND(OUTPUT ${GEN_SOURCES} ${GEN_HEADERS}
        COMMENT "Running rtiddsgen (FACE3C++) for perftest.idl"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${RTITSSHOME}/bin/rtiddsgen
                -d ${CMAKE_CURRENT_SOURCE_DIR}
                -I ${RTITSSHOME}/include
                -replace ${RTIDDSGEN_OPTS}
                -D RTI_PERF_TSS
                -language FACE3C++
                ${CMAKE_CURRENT_SOURCE_DIR}/../srcIdl/perftest.idl)


set(TSS_CONFIG_SRC ${CMAKE_CURRENT_SOURCE_DIR}/perftest_QosSupport.c
                   ${CMAKE_CURRENT_SOURCE_DIR}/perftest_Config.c)

ADD_CUSTOM_COMMAND(OUTPUT ${TSS_CONFIG_SRC}
        DEPENDS ${GEN_SOURCES} ${GEN_HEADERS}
        COMMENT "Copying TSS Config and QoS"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ../resource/tss/ .)

# Example application
set(APP_NAME perftest_cpp_tss_${RTI_CONNEXT_TYPE})
add_executable(${APP_NAME})

add_definitions( -DRTI_LANGUAGE_CPP_TRADITIONAL -O3 -DRTI_PERF_TSS )

if (${RTI_CONNEXT_TYPE} MATCHES ${RTI_CONNEXT_PRO_TYPE})
    add_definitions( -DRTI_PERF_PRO )
else()
    add_definitions(-DRTI_PERF_MICRO )
endif()

add_definitions()

list(APPEND PERFTEST_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/perftest_publisher.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/ThreadPriorities.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/Parameter.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/ParameterManager.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/RTITSSImpl.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/CpuMonitor.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/PerftestTransport.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/Infrastructure_${RTI_CONNEXT_TYPE}.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/Infrastructure_common.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/FileDataLoader.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/PerftestPrinter.cxx)

target_sources(${APP_NAME}
        PRIVATE ${TSS_CONFIG_SRC} ${GEN_SOURCES} ${PERFTEST_SOURCE} )

target_include_directories(${APP_NAME}
        PRIVATE ${CONNEXT_INC_DIRS} ${RTITSS_INC_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

if (${RTI_CONNEXT_TYPE} MATCHES ${RTI_CONNEXT_PRO_TYPE})
    target_link_libraries(${APP_NAME}
            PRIVATE ${RTITSSHOME}/lib/${RTI_TSS_PLATFORM}/${RTI_CONNEXT_TYPE}/librti_tsscpp${LIB_SUFFIX}
            PRIVATE ${RTITSSHOME}/lib/${RTI_TSS_PLATFORM}/${RTI_CONNEXT_TYPE}/libftypes${LIB_SUFFIX}
            PRIVATE ${NDDSHOME}/lib/${RTI_TSS_PLATFORM}/libnddscpp${LIB_SUFFIX}
            PRIVATE ${NDDSHOME}/lib/${RTI_TSS_PLATFORM}/libnddsc${LIB_SUFFIX}
            PRIVATE ${NDDSHOME}/lib/${RTI_TSS_PLATFORM}/libnddscore${LIB_SUFFIX}
            PRIVATE ${ROXMLHOME}/libroxml.a
            PRIVATE ${SYSLIBS_ARCH})
else()
    target_link_libraries(${APP_NAME}
            PRIVATE ${RTITSSHOME}/lib/${RTI_TSS_PLATFORM}/${RTI_CONNEXT_TYPE}/librti_tsscpp${LIB_SUFFIX}
            PRIVATE ${RTITSSHOME}/lib/${RTI_TSS_PLATFORM}/${RTI_CONNEXT_TYPE}/libftypes${LIB_SUFFIX}
            PRIVATE ${RTIMEHOME}/lib/${RTI_TSS_PLATFORM}/librti_me${LIB_SUFFIX}
            PRIVATE ${RTIMEHOME}/lib/${RTI_TSS_PLATFORM}/librti_me_whsm${LIB_SUFFIX}
            PRIVATE ${RTIMEHOME}/lib/${RTI_TSS_PLATFORM}/librti_me_discdpde${LIB_SUFFIX}
            PRIVATE ${SYSLIBS_ARCH})
endif()

set(APP_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../bin/${RTI_TSS_PLATFORM}/${CMAKE_BUILD_TYPE})
add_custom_command(TARGET ${APP_NAME}
                    POST_BUILD
                        COMMENT "Copying application to ${APP_OUTPUT_DIR}"
                        COMMAND ${CMAKE_COMMAND} -E make_directory ${APP_OUTPUT_DIR}
                        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${APP_NAME}> ${APP_OUTPUT_DIR})
