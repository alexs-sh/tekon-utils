set(SYNC_SRC check.c)

add_library(libsync OBJECT ${SYNC_SRC})

add_executable(tekon_sync $<TARGET_OBJECTS:libtekon> 
                          $<TARGET_OBJECTS:libutils>
                          $<TARGET_OBJECTS:libsync>
                          main.c)

# Подключить тесты
if (${TEKON_TESTS_ON})
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
endif()

# Установка утилит
install(TARGETS tekon_sync RUNTIME DESTINATION bin)



