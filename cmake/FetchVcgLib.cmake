include(FetchContent)

FetchContent_Declare(
  vcglib
  GIT_REPOSITORY https://github.com/cnr-isti-vclab/vcglib
  GIT_TAG 2022.02
)

FetchContent_MakeAvailable(vcglib)

set_property(TARGET vcglib_ide PROPERTY FOLDER "External")
