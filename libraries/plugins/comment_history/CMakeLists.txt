file(GLOB HEADERS "include/steemit/comment_history/*.hpp")

add_library( steemit_comment_history
             comment_history_plugin.cpp
             comment_history_api.cpp
           )

target_link_libraries( steemit_comment_history steemit_chain steemit_app )
target_include_directories( steemit_comment_history
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   steemit_comment_history

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
