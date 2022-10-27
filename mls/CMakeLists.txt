add_library(mls)

target_link_libraries(mls PRIVATE mscharconv)
target_link_libraries(mls PRIVATE spdlog::spdlog)
target_link_libraries(mls PRIVATE magic_enum::magic_enum)
target_link_libraries(mls PRIVATE http)

target_sources(mls
    PUBLIC
        csv.h
        database.h
        processor.h
        radio.h
        record.h
        record_set.h
    PRIVATE
        csv.cpp
        database.cpp
        processor.cpp
        record.cpp
        record_set.cpp
)
