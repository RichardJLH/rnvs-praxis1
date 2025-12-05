add_library(util string_view.c failure.c)
target_include_directories(util PUBLIC ${CMAKE_SOURCE_DIR})
