set (TEKON_SRC  message.c 
                pack.c
                unpack.c
                proto.c
                time.c)

# Объектные файлы для внетреннего использования (тесты и примеры)
add_library(libtekon OBJECT ${TEKON_SRC})

# Подключить тесты
if (${TEKON_TESTS_ON})
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
endif()
