#工程所需最小版本号
cmake_minimum_required(VERSION 3.10)

project(multiple-server)
#调试 Debug Release
set(CMAKE_BUILD_TYPE "Debug")

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

#设置搜索路径
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

#找到SDL2_net库
find_package(SDL2 REQUIRED)
find_package(SDL2_net REQUIRED)

#添加对应的头文件搜索目录
include_directories(${SDL2_NET_INCLUDE_DIR})
#生成可执行文件
set(COMMON_LIST "./StringUtils.cpp;./tcputil.cpp")
aux_source_directory(. SRC_LIST)
message(${SRC_LIST})
message(${COMMON_LIST})

add_executable(server "${COMMON_LIST};./server.cpp;./TCPServer.cpp")
#链接对应的函数库
target_link_libraries(server
	${SDL2_NET_LIBRARY}
	${SDL2_LIBRARY})

add_executable(client "${COMMON_LIST};./client.cpp")
#链接对应的函数库
target_link_libraries(client
	${SDL2_NET_LIBRARY}
	${SDL2_LIBRARY})
#设置生成路径在源路径下
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
