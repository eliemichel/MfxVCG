# The easiest way to include the OpenMfx SDK is by using FetchContent as follows,
# you may also copy the repository and call add_subdirectory(OpenMfx).

include(FetchContent)

FetchContent_Declare(
  OpenMfx
  GIT_REPOSITORY https://github.com/eliemichel/OpenMfx.git
  GIT_TAG dev # adapt here to the version you want to use
)

FetchContent_MakeAvailable(OpenMfx)
