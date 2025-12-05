add_library(http parse.c packet.c respond.c)
target_include_directories(http PUBLIC ${CMAKE_SOURCE_DIR})
