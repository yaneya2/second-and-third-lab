cmake_minimum_required(VERSION 4.0)
project(secondlab)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

add_subdirectory(googletest)

add_executable(secondlab main.cpp
        classes/headers/Sequence.h
        classes/headers/ArraySequence.h
        classes/headers/ListSequence.h
        classes/headers/DynamicArray.h
        classes/headers/LinkedList.h
        classes/headers/MutableArraySequence.h
        classes/headers/ImmutableArraySequence.h
        classes/headers/MutableListSequence.h
        classes/headers/MutableListSequence.h
        classes/headers/Option.h
        classes/headers/IEnumerator.h
        classes/headers/MutableArraySequence.h
        classes/headers/ImmutableListSequence.h
        classes/headers/UI.h
        classes/cpp/UI.cpp
        classes/cpp/BItSequence.cpp
        classes/headers/BItSequence.h
        classes/headers/DoublyLinkedList.h
        classes/headers/SegmentedDeque.h
        classes/headers/Student.h
        classes/headers/Teacher.h
        classes/cpp/UIDeque.cpp
        classes/headers/UIDeque.h

)

enable_testing()
add_executable(tests
        tests/testsDynamicArray.cpp
        tests/testsLinkedList.cpp
        tests/testsSequence.cpp
        tests/testsSegmentedDeque.cpp
)

target_link_libraries(tests PRIVATE GTest::gtest_main)

set(SFML_DIR "D:/SFML-3.1.0/lib/cmake/SFML")
find_package(SFML 3.1 REQUIRED COMPONENTS Graphics Window System)
target_link_libraries(secondlab PRIVATE SFML::Graphics SFML::Window SFML::System)
