cmake_minimum_required(VERSION 2.8)

project(final)

find_package(Threads REQUIRED)

add_compile_options(-std=c++17)

add_executable(final main.cpp threadpool.h sock.h epoll.h daemonize.h handleHttpRequest.h)

if (THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(final PUBLIC "-pthread")
endif()
if (CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(final "${CMAKE_THREAD_LIBS_INIT}")
endif()
