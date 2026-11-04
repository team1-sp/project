find_package(Protobuf CONFIG QUIET)
if(NOT Protobuf_FOUND)
    find_package(Protobuf REQUIRED)
endif()
find_package(gRPC CONFIG REQUIRED)
find_package(flatbuffers CONFIG REQUIRED)
find_package(cppzmq CONFIG REQUIRED)
find_path(ZEROMQ_INCLUDE_DIR NAMES zmq.h REQUIRED)
find_library(ZEROMQ_LIBRARY NAMES zmq libzmq REQUIRED)
add_library(bridge_benchmark_zeromq UNKNOWN IMPORTED)
set_target_properties(bridge_benchmark_zeromq PROPERTIES
    IMPORTED_LOCATION "${ZEROMQ_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${ZEROMQ_INCLUDE_DIR}"
)

set(PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/proto/agent_bridge.proto")
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(PROTO_SOURCE "${GENERATED_DIR}/agent_bridge.pb.cc")
set(PROTO_HEADER "${GENERATED_DIR}/agent_bridge.pb.h")
set(GRPC_SOURCE "${GENERATED_DIR}/agent_bridge.grpc.pb.cc")
set(GRPC_HEADER "${GENERATED_DIR}/agent_bridge.grpc.pb.h")
set(FLATBUFFERS_SCHEMA "${CMAKE_CURRENT_SOURCE_DIR}/flatbuffers/agent_bridge.fbs")
set(FLATBUFFERS_HEADER "${GENERATED_DIR}/agent_bridge_generated.h")

add_custom_command(
    OUTPUT "${PROTO_SOURCE}" "${PROTO_HEADER}" "${GRPC_SOURCE}" "${GRPC_HEADER}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GENERATED_DIR}"
    COMMAND protobuf::protoc
        --proto_path ${CMAKE_CURRENT_SOURCE_DIR}/proto
        --cpp_out ${GENERATED_DIR}
        --grpc_out ${GENERATED_DIR}
        --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
        ${PROTO_FILE}
    DEPENDS "${PROTO_FILE}"
    VERBATIM
)

add_custom_command(
    OUTPUT "${FLATBUFFERS_HEADER}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GENERATED_DIR}"
    COMMAND flatbuffers::flatc --cpp -o "${GENERATED_DIR}" "${FLATBUFFERS_SCHEMA}"
    DEPENDS "${FLATBUFFERS_SCHEMA}"
    VERBATIM
)
add_custom_target(bridge_benchmark_flatbuffers_generated DEPENDS "${FLATBUFFERS_HEADER}")

add_library(bridge_benchmark_messages STATIC "${PROTO_SOURCE}")
target_include_directories(bridge_benchmark_messages PUBLIC "${GENERATED_DIR}")
target_link_libraries(bridge_benchmark_messages PUBLIC protobuf::libprotobuf)

add_library(bridge_benchmark_grpc STATIC "${GRPC_SOURCE}")
target_include_directories(bridge_benchmark_grpc PUBLIC "${GENERATED_DIR}")
target_link_libraries(bridge_benchmark_grpc PUBLIC bridge_benchmark_messages gRPC::grpc++)

add_executable(grpc_bridge_benchmark cpp/grpc_client.cpp)
target_include_directories(grpc_bridge_benchmark PRIVATE cpp)
target_link_libraries(grpc_bridge_benchmark PRIVATE bridge_benchmark_grpc)
engine_set_warnings(grpc_bridge_benchmark)

add_executable(zmq_bridge_benchmark cpp/zmq_client.cpp)
target_include_directories(zmq_bridge_benchmark PRIVATE cpp)
target_link_libraries(zmq_bridge_benchmark PRIVATE bridge_benchmark_messages bridge_benchmark_zeromq)
engine_set_warnings(zmq_bridge_benchmark)

add_executable(zmq_flatbuffers_bridge_benchmark
    cpp/zmq_flatbuffers_client.cpp
    "${FLATBUFFERS_HEADER}"
)
add_dependencies(zmq_flatbuffers_bridge_benchmark bridge_benchmark_flatbuffers_generated)
target_include_directories(zmq_flatbuffers_bridge_benchmark PRIVATE cpp "${GENERATED_DIR}")
target_link_libraries(zmq_flatbuffers_bridge_benchmark PRIVATE
    cppzmq
    flatbuffers::flatbuffers
)
engine_set_warnings(zmq_flatbuffers_bridge_benchmark)

if(NOT AEGIS_BRIDGE_PYTHON_EXECUTABLE)
    find_package(Python 3.12 EXACT COMPONENTS Interpreter REQUIRED)
    set(AEGIS_BRIDGE_PYTHON_EXECUTABLE "${Python_EXECUTABLE}")
endif()
set(Python_EXECUTABLE "${AEGIS_BRIDGE_PYTHON_EXECUTABLE}" CACHE FILEPATH "" FORCE)
find_package(Python 3.12 EXACT COMPONENTS Interpreter Development.Module REQUIRED)
set(PYBIND11_FINDPYTHON ON)
# pybind11 caches these legacy compatibility values. Clear them so changing the configured
# interpreter cannot leave an extension suffix from a different Python ABI.
unset(PYTHON_MODULE_EXTENSION CACHE)
unset(PYTHON_MODULE_DEBUG_POSTFIX CACHE)
execute_process(
    COMMAND "${AEGIS_BRIDGE_PYTHON_EXECUTABLE}" -m pybind11 --cmakedir
    OUTPUT_VARIABLE PYBIND11_CMAKE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE PYBIND11_CMAKE_RESULT
)
if(NOT PYBIND11_CMAKE_RESULT EQUAL 0)
    message(FATAL_ERROR "pybind11 is required in AEGIS_BRIDGE_PYTHON_EXECUTABLE")
endif()
list(APPEND CMAKE_PREFIX_PATH "${PYBIND11_CMAKE_DIR}")
find_package(pybind11 CONFIG REQUIRED)
pybind11_add_module(aegis_bridge_pybind MODULE cpp/pybind_module.cpp)
set_target_properties(aegis_bridge_pybind PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/python"
)
engine_set_warnings(aegis_bridge_pybind)

if(ENGINE_BUILD_TESTS)
    add_test(
        NAME bridge_benchmark_integration
        COMMAND "${AEGIS_BRIDGE_PYTHON_EXECUTABLE}" -m pytest -q
            "${CMAKE_CURRENT_SOURCE_DIR}/python/tests"
    )
    set_tests_properties(bridge_benchmark_integration PROPERTIES
        ENVIRONMENT
            "PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR}/python;BRIDGE_BENCH_BUILD_DIR=${CMAKE_CURRENT_BINARY_DIR};BRIDGE_BENCH_PYBIND_DIR=${CMAKE_CURRENT_BINARY_DIR}/python"
        TIMEOUT 90
    )
endif()
