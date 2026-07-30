function(deoxy_enable_warnings TARGET_NAME)
    target_compile_options("${TARGET_NAME}"
        PRIVATE
            $<$<CXX_COMPILER_ID:GNU,Clang>:
                -Wall
                -Wextra
                -Wpedantic
                -Wshadow
            >
            $<$<CXX_COMPILER_ID:MSVC>:
                /W4
                /permissive-
            >
    )
endfunction()
