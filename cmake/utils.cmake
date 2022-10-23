# A simple utility function to treat warnings as errors
function(target_treat_warnings_as_errors target)
	if(MSVC)
		target_compile_options(${target} PRIVATE /W4 /WX)
	else()
		target_compile_options(${target} PRIVATE -Wall -Wextra -pedantic -Werror)
	endif()
endfunction()

macro(add_openmfx_plugin Target)
  set(options TREAT_WARNINGS_AS_ERRORS)
  set(oneValueArgs)
  set(multiValueArgs SRC LIBS)
  cmake_parse_arguments("" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # An OpenMfx plugin is a shared library with a different extension
  add_library(${Target} SHARED ${_SRC})
  set_target_properties(${Target} PROPERTIES SUFFIX ".ofx")

  target_link_libraries(${Target} PRIVATE ${_LIBS})

  if (DEFINED _TREAT_WARNINGS_AS_ERRORS)
    target_link_libraries(${Target} PRIVATE OpenMfx::Sdk::Cpp::Plugin)
  endif()
endmacro()
