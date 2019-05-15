set(ARCH_SRC unit_check.c)

add_executable(unit_check  $<TARGET_OBJECTS:libtekon> 
                          $<TARGET_OBJECTS:libutils> 
                          $<TARGET_OBJECTS:libsync> 
                          ${ARCH_SRC})

add_test(unit_utils_sync_check ${CMAKE_CURRENT_BINARY_DIR}/unit_check)


