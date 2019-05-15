set(LINK_SRC link.c)
set(READ_MSR_SRC read_msr.c)

add_executable(app_link $<TARGET_OBJECTS:libtekon>
                        $<TARGET_OBJECTS:libutils> 
                        ${LINK_SRC})

add_executable(app_read_msr $<TARGET_OBJECTS:libtekon>
                            $<TARGET_OBJECTS:libutils>
                            ${READ_MSR_SRC})

