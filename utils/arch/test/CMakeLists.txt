set(ARCH_SRC unit_arch.c)

add_executable(unit_arch  $<TARGET_OBJECTS:libtekon> 
                          $<TARGET_OBJECTS:libutils> 
                          $<TARGET_OBJECTS:libarch> 
                          ${ARCH_SRC})
add_test(unit_utils_arch_arch ${CMAKE_CURRENT_BINARY_DIR}/unit_arch)


