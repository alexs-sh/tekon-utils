set(MSR_SRC msr.c)

add_library(libmsr OBJECT ${MSR_SRC})
add_executable(tekon_msr  $<TARGET_OBJECTS:libtekon> 
                          $<TARGET_OBJECTS:libutils>
                          $<TARGET_OBJECTS:libmsr>
                          main.c)

# Подключить тесты
if (${TEKON_TESTS_ON})
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
endif()

# Установка утилит
install(TARGETS tekon_msr RUNTIME DESTINATION bin)



