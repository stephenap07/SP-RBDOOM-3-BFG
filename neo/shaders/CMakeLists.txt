include(../compileshaders.cmake)

file(GLOB shaders "*.hlsl"
                  "renderprogs/*.hlsl")

set(project Shaders)
set(SHADER_INCLUDE_DIR "shaders/")

compile_shaders_all_platforms(
    TARGET Shaders
    CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/shaders.cfg
    OUTPUT_BASE ${CMAKE_CURRENT_SOURCE_DIR}/../../base/renderprogs/ # This path scheme is a bit odd.
    SOURCES ${shaders}
)