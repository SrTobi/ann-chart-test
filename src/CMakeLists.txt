

SCAN_SOURCE_HERE(SOURCE "cpp;hpp")
AUTO_SOURCE_GROUP("${SOURCE}")

add_executable(ai-test ${SOURCE})
target_link_libraries(ai-test ${Boost_LIBRARIES} ${SFML_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})