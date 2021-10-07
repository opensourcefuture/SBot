# 指定CMake最低版本
cmake_minimum_required(VERSION 3.14)

# 指定工程名字
project (SBot VERSION 1.0)

# 设置c++标准
set(CMAKE_CXX_STANDARD 17)

# 当前不支持MSVC编译
if(MSVC)
    message(FATAL_ERROR "${PROJECT_NAME} DO NOT SUPPORT BUILD BY MSVC")
endif()

# 设置编译方式
if(NOT ${PROJECT_NAME}_LIBRARY_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(${PROJECT_NAME}_LIBRARY_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE ${PROJECT_NAME}_LIBRARY_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# 指定工程类型(共享库)以及源文件
add_library(${PROJECT_NAME} 
    SHARED 
        src/SBot_Core.cpp
        src/SBot_Ws_Impl.cpp
        src/SBot_Def.cpp
        src/SBot_Help.cpp
        src/SBot_Tools.cpp
)

# 指定头文件
target_include_directories(${PROJECT_NAME}
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# 使没有 __declspec(dllexport) 修饰的函数不导出
if(WIN32)
    set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS " -Wl,--exclude-all-symbols ")
else()
    add_definitions(-fPIC)
    set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS " -fvisibility=hidden ")
    target_link_libraries(${PROJECT_NAME} PRIVATE -pthread)
endif()

# 导入CPM脚本
include(cmake/CPM.cmake)

# 导入asio依赖
CPMAddPackage(
    NAME asio	 
    GIT_TAG master
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio
    DOWNLOAD_ONLY YES
)
target_include_directories(${PROJECT_NAME} PRIVATE ${asio_SOURCE_DIR}/asio/include)

# 导入websocketpp依赖
CPMAddPackage(
    NAME websocketpp	 
    GIT_TAG master
    GIT_REPOSITORY https://github.com/zaphoyd/websocketpp
    DOWNLOAD_ONLY YES
)
target_include_directories(${PROJECT_NAME} PRIVATE ${websocketpp_SOURCE_DIR})

# 如果是windows平台，则链接ws2_32
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

# 导入jsoncpp依赖
include(cmake/jsoncpp.cmake)
target_link_libraries(${PROJECT_NAME} PRIVATE jsoncpp_static)

# install
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    SBotConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion
)

set_target_properties(SBot PROPERTIES PUBLIC_HEADER ${CMAKE_SOURCE_DIR}/include/SBot_Core.h)
install(TARGETS SBot
        EXPORT SBotTargets 
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include
)
install(
        EXPORT SBotTargets
        FILE SBotTargets.cmake
        DESTINATION lib/cmake/SBot
)
configure_file(SBotConfig.cmake.in SBotConfig.cmake @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/SBotConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/SBotConfigVersion.cmake"
            DESTINATION lib/cmake/SBot
)

add_executable(testbot 
    test/testbot.c)
target_link_libraries(testbot PRIVATE SBot)
target_link_libraries(testbot PRIVATE jsoncpp_static)

add_executable(testuuid
    test/testuuid.cpp
    src/SBot_Tools.cpp
)
target_include_directories(testuuid PRIVATE include)



