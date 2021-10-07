# 导入CPM脚本
include(cmake/CPM.cmake)

# 导入base64依赖
CPMAddPackage(
    NAME base64	 
    GIT_TAG master
    GIT_REPOSITORY https://github.com/ReneNyffenegger/cpp-base64
    DOWNLOAD_ONLY YES
)

add_library(base64
    STATIC 
        ${base64_SOURCE_DIR}/base64.cpp
)

# 指定头文件
target_include_directories(base64
    PUBLIC 
        ${base64_SOURCE_DIR}
)
