cmake_minimum_required(VERSION 3.20)
project(test_pthread LANGUAGES CXX)

# ���������� �������� C++
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# �������� ���������� � ������������� �������
include_directories(${PROJECT_SOURCE_DIR}/include)

# ���������
set(SOURCES
    src/main.cpp
)

# �������� ���������-��������� �����
if (WIN32)
    list(APPEND SOURCES src/tcp_socket_win.cpp)
    set(LIBS ws2_32)
else()
    list(APPEND SOURCES src/tcp_socket_posix.cpp)
    set(LIBS)
endif()

# �������� ����������� ����
add_executable(test_pthread ${SOURCES})

target_link_libraries(test_pthread ${LIBS})
