set(LINK_SRC unit_link.c)
set(TYPES_SRC unit_types.c)
set(TIME_SRC unit_time.c)
set(TSTAMP_SRC unit_tstamp.c)

# Общие тесты
add_executable(unit_types $<TARGET_OBJECTS:libtekon> 
                            $<TARGET_OBJECTS:libutils> 
                            ${TYPES_SRC})

add_executable(unit_time $<TARGET_OBJECTS:libtekon> 
                         $<TARGET_OBJECTS:libutils> 
                         ${TIME_SRC})

add_executable(unit_tstamp $<TARGET_OBJECTS:libtekon> 
                         $<TARGET_OBJECTS:libutils> 
                         ${TSTAMP_SRC})


add_test(unit_utils_base_types ${CMAKE_CURRENT_BINARY_DIR}/unit_types)
add_test(unit_utils_base_time ${CMAKE_CURRENT_BINARY_DIR}/unit_time)
add_test(unit_utils_base_tstamp ${CMAKE_CURRENT_BINARY_DIR}/unit_tstamp)

# Тесты, специфичные для ОС
if (${TEKON_TARGET_OS} STREQUAL "Linux")
  add_executable(unit_link $<TARGET_OBJECTS:libtekon> 
                           $<TARGET_OBJECTS:libutils> 
                           ${LINK_SRC})
  add_test(unit_utils_base_link ${CMAKE_CURRENT_BINARY_DIR}/unit_link)
else()
  # NOOP
endif()

