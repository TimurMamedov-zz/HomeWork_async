cmake_minimum_required(VERSION 3.9)

if(DEFINED ENV{TRAVIS_BUILD_NUMBER})
    project(async_app VERSION 0.0.$ENV{TRAVIS_BUILD_NUMBER})
else()
    project(async_app VERSION 0.0.1)
endif()

find_package(Threads)
#set(Boost_USE_STATIC_LIBS ON)
#set(Boost_USE_STATIC_RUNTIME ON)
#find_package(Boost COMPONENTS unit_test_framework program_options REQUIRED)

configure_file(version.h.in version.h)

set(HEADERS commands_storage.h threadsafe_queue.h solver.h solvers.h async.h)
set(SOURCES commands_storage.cpp solvers.cpp async.cpp)

add_library(async SHARED
    ${HEADERS}
    ${SOURCES}
)

add_executable(async_app async.h main.cpp)

#set (TEST test_bulk)
#add_executable (${TEST} test_bulk.cpp)

set_target_properties (async_app async PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
        COMPILE_OPTIONS -Wpedantic -Wall -Wextra
)

#set_target_properties(bulk ${TEST} PROPERTIES
#    COMPILE_DEFINITIONS BOOST_TEST_STATIC_LINK
#    INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
#)

#target_link_libraries(bulk
#        ${Boost_PROGRAM_OPTIONS_LIBRARY}
#)

#target_link_libraries(${TEST}
#        ${Boost_LIBRARIES}
#)

target_link_libraries(async
  ${CMAKE_THREAD_LIBS_INIT}
  )

target_link_libraries(async_app
  async
  )

install(TARGETS async RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT tima_mamedxp@mail.ru)

include(CPack)

#enable_testing()

#add_test (${TEST} ${TEST})
