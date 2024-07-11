cmake_minimum_required(VERSION 3.20)
project(test_pthread LANGUAGES CXX)

# Установите стандарт C++
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавьте директорию с заголовочными файлами
include_directories(${PROJECT_SOURCE_DIR}/include)

# Источники
set(SOURCES
    src/main.cpp
)

# Добавьте платформо-зависимые файлы
if (WIN32)
    list(APPEND SOURCES src/tcp_socket_win.cpp)
    set(LIBS ws2_32)
else()
    list(APPEND SOURCES src/tcp_socket_posix.cpp)
    set(LIBS)
endif()

# Создайте исполняемый файл
add_executable(test_pthread ${SOURCES})

target_link_libraries(test_pthread ${LIBS})
