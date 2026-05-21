cmake_minimum_required(VERSION 3.10)

project(MTree VERSION 1.0)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(WIN32)
    message(STATUS "Configurando para Windows")
elseif(APPLE)
    message(STATUS "Configurando para macOS")
elseif(UNIX)
    message(STATUS "Configurando para Linux/Unix")
else()
    message(WARNING "Sistema operativo desconocido")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    message(STATUS "Usando el compilador GCC")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Usando el compilador Clang")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    message(STATUS "Usando el compilador Microsoft Visual Studio")
else()
    message(WARNING "Compilador desconocido: ${CMAKE_CXX_COMPILER_ID}")
endif()


if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -pedantic)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W4)
endif()

# Incluir directorios
include_directories(include)

# Archivos fuente
file(GLOB SOURCES "src/*.cpp")

# Crear el ejecutable
add_executable(MTree ${SOURCES})
