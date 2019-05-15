set(PACK_SRC unit_pack.c)
set(UNPACK_SRC unit_unpack.c)
set(TIME_SRC unit_time.c)
set(PROTO_SRC unit_proto.c)

add_executable(unit_tekon_pack $<TARGET_OBJECTS:libtekon> ${PACK_SRC})
add_executable(unit_tekon_unpack $<TARGET_OBJECTS:libtekon> ${UNPACK_SRC})
add_executable(unit_tekon_time $<TARGET_OBJECTS:libtekon> ${TIME_SRC})
add_executable(unit_tekon_proto $<TARGET_OBJECTS:libtekon> ${PROTO_SRC})

add_test(unit_tekon_pack ${CMAKE_CURRENT_BINARY_DIR}/unit_tekon_pack)
add_test(unit_tekon_unpack ${CMAKE_CURRENT_BINARY_DIR}/unit_tekon_unpack)
add_test(unit_tekon_time ${CMAKE_CURRENT_BINARY_DIR}/unit_tekon_time)
add_test(unit_tekon_proto ${CMAKE_CURRENT_BINARY_DIR}/unit_tekon_proto)


