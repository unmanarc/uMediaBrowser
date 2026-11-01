cmake_minimum_required(VERSION 3.12)
include(GNUInstallDirs)

set(APP_NAME "uMediaBrowser")
set(SVERSION 3.0.0)

project(${APP_NAME})
project(${PROJECT_NAME} VERSION ${SVERSION} DESCRIPTION "Unmanarc's Media Browser")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

configure_file(src/config.h.in ${${APP_NAME}_SOURCE_DIR}/src/config.h)

file(GLOB_RECURSE EDV_INCLUDE_FILES "src/*.h*")
file(GLOB_RECURSE EDV_SOURCE_FILES "src/*.c*")

add_executable( ${APP_NAME} ${EDV_INCLUDE_FILES} ${EDV_SOURCE_FILES} )

install( TARGETS ${APP_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )

################################################################################
# Mantids30 location:
#   - Using pkg-config (installed libMantids30)
#   - MANTIDS30_PREFIX or EXTRAPREFIX can point to the installation prefix
#     (eg. /home/user/Devel/ROOT)
################################################################################
find_package(PkgConfig REQUIRED)

if (MANTIDS30_PREFIX)
    target_include_directories(${APP_NAME} PUBLIC ${MANTIDS30_PREFIX}/include)
    target_link_libraries(${APP_NAME} "-L${MANTIDS30_PREFIX}/lib")
    set(ENV{PKG_CONFIG_PATH} "${MANTIDS30_PREFIX}/share/pkgconfig:${MANTIDS30_PREFIX}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
endif()

if (EXTRAPREFIX)
    target_include_directories(${APP_NAME} PUBLIC ${EXTRAPREFIX}/include)
    target_link_libraries(${APP_NAME} "-L${EXTRAPREFIX}/lib")
    target_link_libraries(${APP_NAME} "-L${EXTRAPREFIX}/lib64")
    set(ENV{PKG_CONFIG_PATH} "${EXTRAPREFIX}/share/pkgconfig:${EXTRAPREFIX}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
endif()

if (CMAKE_INSTALL_PREFIX)
    target_include_directories(${APP_NAME} PUBLIC ${CMAKE_INSTALL_PREFIX}/include)
    target_include_directories(${APP_NAME} PUBLIC ${CMAKE_INSTALL_PREFIX}/usr/include)
    target_link_libraries(${APP_NAME} "-L${CMAKE_INSTALL_PREFIX}/lib")
    target_link_libraries(${APP_NAME} "-L${CMAKE_INSTALL_PREFIX}/lib64")
endif()

################################################################################
# Dependencies:
################################################################################
pkg_check_modules(JSONCPP REQUIRED jsoncpp)
target_include_directories(${APP_NAME} PUBLIC ${JSONCPP_INCLUDE_DIRS})
target_link_libraries(${APP_NAME} ${JSONCPP_LIBRARIES})

find_package(Boost REQUIRED COMPONENTS regex thread)
target_include_directories(${APP_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(${APP_NAME} ${Boost_LIBRARIES})

# std::filesystem needs an explicit library on older GCC toolchains.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    target_link_libraries(${APP_NAME} "-lstdc++fs")
endif()

################################################################################
# Mantids30 Libraries:
################################################################################
set(Mantids30_LIBRARIES
    Mantids30_Protocol_HTTP
    Mantids30_Protocol_MIME
    Mantids30_Net_Sockets
    Mantids30_Threads
    Mantids30_Memory
    Mantids30_Program_Service
    Mantids30_Program_Logs
    Mantids30_Helpers
)

foreach(LIBRARY ${Mantids30_LIBRARIES})
    pkg_check_modules(${LIBRARY} REQUIRED ${LIBRARY})
    target_include_directories(${APP_NAME} PUBLIC ${${LIBRARY}_INCLUDE_DIRS})
    target_link_libraries(${APP_NAME} ${${LIBRARY}_LDFLAGS})
endforeach()

target_link_libraries(${APP_NAME} "-lpthread")
target_link_libraries(${APP_NAME} "-ldl")

if (EXTRAPREFIX)
    set_target_properties(${APP_NAME} PROPERTIES BUILD_RPATH "${EXTRAPREFIX}/lib")
    # Use transitive RPATH so dependencies of Mantids30 libraries are found too.
    target_link_options(${APP_NAME} PRIVATE "-Wl,--disable-new-dtags")
endif()

if (WITH_SSL_SUPPORT)
    add_definitions(-DWITH_SSL_SUPPORT)

    option(SSLRHEL7 "OpenSSL 1.1 For Red Hat 7.x provided by EPEL" OFF)
    if (SSLRHEL7)
        pkg_check_modules(OPENSSL REQUIRED libssl11)
    else()
        pkg_check_modules(OPENSSL REQUIRED libssl)
    endif()
    target_include_directories(${APP_NAME} PUBLIC ${OPENSSL_INCLUDE_DIRS})
    target_compile_options(${APP_NAME} PUBLIC ${OPENSSL_CFLAGS_OTHER})
    target_link_libraries(${APP_NAME} ${OPENSSL_LDFLAGS})
    target_link_libraries(${APP_NAME} "-lz")
endif()

if (WIN32)
    target_link_libraries(${APP_NAME} "-lws2_32")
endif()
