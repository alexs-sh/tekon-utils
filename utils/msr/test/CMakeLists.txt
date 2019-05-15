set(MSR_SRC unit_msr.c)

add_executable(unit_msr $<TARGET_OBJECTS:libmsr>
                        $<TARGET_OBJECTS:libtekon> 
                        $<TARGET_OBJECTS:libutils> 
                        ${MSR_SRC})

add_test(unit_utils_msr_msr ${CMAKE_CURRENT_BINARY_DIR}/unit_msr)


