if(UNIX AND NOT APPLE)
    execute_process(
        COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET
        OUTPUT_VARIABLE ld_version
    )
    if("${ld_version}" MATCHES "GNU gold")
        option(USE_GOLD_LINKER "Wheather the gold linker should be used" OFF)
        if(USE_GOLD_LINKER)
            set(CMAKE_EXE_LINKER_FLAGS
                "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags"
            )
            set(CMAKE_SHARED_LINKER_FLAGS
                "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags"
            )
        endif()
    endif()
endif()
