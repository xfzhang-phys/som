# load triqs helper to set up tests
find_package(TriqsTest)

# Simple tests
file(GLOB test_sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

foreach(filename ${test_sources})
    get_filename_component(t ${filename} NAME_WE)
    add_executable(${t} ${CMAKE_CURRENT_SOURCE_DIR}/${filename})
    target_link_libraries(${t} som_c ${TRIQS_LIBRARY_ALL} ${TRIQS_LIBRARY_GTEST})
    triqs_set_rpath_for_target(${t})
    triqs_add_cpp_test(${t})
endforeach()

# Copy h5 files with input data
file(GLOB h5_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h5)
file(COPY ${h5_files} DESTINATION .)

add_subdirectory(numerics)
