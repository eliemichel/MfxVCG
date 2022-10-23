
file(GLOB FetchFiles ${CMAKE_CURRENT_LIST_DIR}/Fetch*.cmake)

foreach(f ${FetchFiles})
	include(${f})
endforeach()
