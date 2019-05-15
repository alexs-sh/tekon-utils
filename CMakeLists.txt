cmake_minimum_required(VERSION 3.1)
project(tekon-utils C)

# Определить хостовую и целевую системы
set (TEKON_HOST_OS ${CMAKE_SYSTEM_NAME} CACHE STRING "Host OS name")

if(NOT TEKON_TARGET_OS)
  message("Target OS not specified. Use default")
  set(TEKON_TARGET_OS ${TEKON_HOST_OS} CACHE STRING "Target OS name")
endif()

message("Host OS:" ${TEKON_HOST_OS})
message("Target OS:" ${TEKON_TARGET_OS})

# Обязательные флаги для всех типов сборок
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_GNU_SOURCE")
set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -D_GNU_SOURCE")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -D_GNU_SOURCE")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -D_GNU_SOURCE")

# Настройка директорий для включения
include_directories (${CMAKE_CURRENT_SOURCE_DIR})

# Настройка тестов 
option (TEKON_TESTS_ON "Build tests" ON)

if (${TEKON_TESTS_ON})

  enable_testing()

  option (TEKON_TESTS_COVERAGE "Build test with coverage support" OFF)

  if (${TEKON_TESTS_COVERAGE})
    set(CMAKE_BUILD_TYPE "Debug")
    set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall -fprofile-arcs -ftest-coverage -D_GNU_SOURCE")
    message(STATUS "Coverage is ON. Using predefined compiler flags")
    message(STATUS "TYPE: ${CMAKE_BUILD_TYPE}")
    message(STATUS "CFLAGS: ${CMAKE_C_FLAGS_DEBUG}")
  endif()

  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)

endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tekon)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/utils)

