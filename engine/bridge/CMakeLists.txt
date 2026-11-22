add_library(engine_bridge INTERFACE)
add_library(engine::bridge ALIAS engine_bridge)
target_include_directories(engine_bridge INTERFACE include)

if(AEGIS_WITH_GRPC_BRIDGE)
    find_package(Protobuf CONFIG QUIET)
    if(NOT Protobuf_FOUND)
        find_package(Protobuf REQUIRED)
    endif()
    find_package(gRPC CONFIG REQUIRED)

    set(PROTO_FILE "${PROJECT_SOURCE_DIR}/proto/agent_bridge.proto")
    set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
    set(PROTO_SOURCE "${GENERATED_DIR}/agent_bridge.pb.cc")
    set(PROTO_HEADER "${GENERATED_DIR}/agent_bridge.pb.h")
    set(GRPC_SOURCE "${GENERATED_DIR}/agent_bridge.grpc.pb.cc")
    set(GRPC_HEADER "${GENERATED_DIR}/agent_bridge.grpc.pb.h")

    add_custom_command(
        OUTPUT "${PROTO_SOURCE}" "${PROTO_HEADER}" "${GRPC_SOURCE}" "${GRPC_HEADER}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${GENERATED_DIR}"
        COMMAND protobuf::protoc
            --proto_path "${PROJECT_SOURCE_DIR}/proto"
            --cpp_out "${GENERATED_DIR}"
            --grpc_out "${GENERATED_DIR}"
            --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
            "${PROTO_FILE}"
        DEPENDS "${PROTO_FILE}"
        VERBATIM
    )

    add_library(engine_bridge_grpc STATIC
        src/grpc_agent_bridge.cpp
        "${PROTO_SOURCE}"
        "${GRPC_SOURCE}"
    )
    add_library(engine::bridge_grpc ALIAS engine_bridge_grpc)
    target_include_directories(engine_bridge_grpc
        PUBLIC include
        PRIVATE "${GENERATED_DIR}"
    )
    target_link_libraries(engine_bridge_grpc
        PUBLIC engine::bridge
        PRIVATE gRPC::grpc++ protobuf::libprotobuf
    )
    engine_set_warnings(engine_bridge_grpc)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # Protobuf 36.2 expands Abseil nullability annotations in generated code. AppleClang
        # diagnoses those annotations as extensions when project warnings are errors.
        target_compile_options(engine_bridge_grpc PRIVATE -Wno-nullability-extension)
    endif()
    if(MSVC)
        # gRPC 1.84 headers use a deprecated destructor (C4996); it is not project code.
        target_compile_options(engine_bridge_grpc PRIVATE /wd4996)
    endif()
endif()
