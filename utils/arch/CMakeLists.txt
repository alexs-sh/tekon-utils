set(ARCH_SRC arch.c)

add_library(libarch OBJECT ${ARCH_SRC})
add_executable(tekon_arch $<TARGET_OBJECTS:libtekon> 
                          $<TARGET_OBJECTS:libutils>
                          $<TARGET_OBJECTS:libarch>
                          main.c)

# Подключить тесты
if (${TEKON_TESTS_ON})
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
endif()

# Установка утилит
install(TARGETS tekon_arch RUNTIME DESTINATION bin)
