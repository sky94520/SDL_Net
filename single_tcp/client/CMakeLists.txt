#工程所需最小版本号
cmake_minimum_required(VERSION 3.10)

project(server)

#设置搜索路径
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake")

#找到SDL2_net库
find_package(SDL2_net REQUIRED)

#添加对应的头文件搜索目录
include_directories(${SDL2_NET_INCLUDE_DIR})
#生成可执行文件
aux_source_directory(. SRC_LIST)
add_executable(main ${SRC_LIST})
#链接对应的函数库
target_link_libraries(main ${SDL2_NET_LIBRARY})
#设置生成路径在源路径下
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
