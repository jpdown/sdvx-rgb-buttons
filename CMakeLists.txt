cmake_minimum_required(VERSION 3.28)
project(sdvx_rgb_buttons)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)
set(SAFETYHOOK_FETCH_ZYDIS ON)
FetchContent_Declare(
        safetyhook
        GIT_REPOSITORY "https://github.com/cursey/safetyhook.git"
        GIT_TAG "629558c64009a7291ba6ed5cfb49187086a27a47" # v0.4.1
)
FetchContent_MakeAvailable(safetyhook)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY "https://github.com/gabime/spdlog"
        GIT_TAG "27cb4c76708608465c413f6d0e6b8d99a4d84302" # v1.14.1
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
        signature-scanner
        GIT_REPOSITORY "https://github.com/jpdown/signature-scanner"
        GIT_TAG "4978f0ef89a08c5f8ff712de413d28c155d83021"
)
FetchContent_MakeAvailable(signature-scanner)

add_library(sdvx_rgb_buttons MODULE library.cpp)
target_link_libraries(${PROJECT_NAME} safetyhook spdlog signature-scanner)
