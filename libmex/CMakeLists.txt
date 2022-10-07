add_library(libmex SHARED libmex.c)
target_include_directories(libmex PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(libmex PROPERTIES
    PREFIX ""
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
