file(GLOB_RECURSE libsrc *.h *.cpp *.hpp *.c *.cc)

set(target_name engine)

add_library(${target_name} ${libsrc} ${shader_files})

include(GenerateExportHeader)
generate_export_header(${target_name})

target_include_directories(${target_name}
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_include_directories(${target_name}
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}/..
)

target_link_libraries (${target_name}
    PUBLIC
    EnTT::EnTT
    Bullet3::Bullet3
    poolSTL::poolSTL
    service
    context
    logging
    cmd_line
    memory
    ospp
    audiopp
    graphics
    simulation
    serialization
    reflection
    filesystem
    math
    uuidpp
    threadpp
    assimp
    string_utils
    monort
    seq
)

target_precompile_headers(${target_name}
    PUBLIC
    pch.h
)

set_target_properties(${target_name} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

