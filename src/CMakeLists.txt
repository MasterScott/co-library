target_sources(co-library PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/UrlEncoding.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/HttpRequest.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/OnlineService.cpp")

target_sources(co-test PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/Main.cpp")