if (${TEKON_TARGET_OS} STREQUAL "Linux")
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/linux)
  set(OS_SPECIFIC_SRC ${CMAKE_CURRENT_SOURCE_DIR}/linux/link.c
                      ${CMAKE_CURRENT_SOURCE_DIR}/linux/time.c)
elseif (${TEKON_TARGET_OS} STREQUAL "Windows") 
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win)
  set(OS_SPECIFIC_SRC ${CMAKE_CURRENT_SOURCE_DIR}/win/link.c
                      ${CMAKE_CURRENT_SOURCE_DIR}/win/time.c)
else()
  message(FATAL_ERROR "Unsupported system ${TEKON_TARGET_OS}")
endif()


set (LIBUTILS_SRC ${OS_SPECIFIC_SRC}
                  types.c
                  tstamp.c
                  log.c
                  string.c
                  )

# Объектные файлы для внетреннего использования (тесты и примеры)
add_library(libutils OBJECT ${LIBUTILS_SRC})


# Подключить тесты
if (${TEKON_TESTS_ON})
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
endif()



