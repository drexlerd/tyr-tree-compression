function(tyr_register_native_dependency_prefix native_prefix)
    if(NOT native_prefix)
        return()
    endif()

    set(native_include_dir_name "${CMAKE_INSTALL_INCLUDEDIR}")
    if(NOT native_include_dir_name)
        set(native_include_dir_name "include")
    endif()

    if(EXISTS "${native_prefix}/${native_include_dir_name}")
        list(APPEND TYR_NATIVE_DEPENDENCY_INCLUDE_DIRECTORIES "${native_prefix}/${native_include_dir_name}")
        set(TYR_NATIVE_DEPENDENCY_INCLUDE_DIRECTORIES "${TYR_NATIVE_DEPENDENCY_INCLUDE_DIRECTORIES}" PARENT_SCOPE)
    endif()
endfunction()

function(tyr_register_python_native_runtime_prefix package_relative_prefix)
    if(NOT package_relative_prefix)
        return()
    endif()

    list(APPEND TYR_PYTHON_NATIVE_RUNTIME_PREFIXES "${package_relative_prefix}")
    set(TYR_PYTHON_NATIVE_RUNTIME_PREFIXES "${TYR_PYTHON_NATIVE_RUNTIME_PREFIXES}" PARENT_SCOPE)
endfunction()

function(tyr_make_python_native_runtime_rpaths output_variable origin package_relative_base)
    set(runtime_rpaths "${origin}")
    foreach(package_relative_prefix IN LISTS TYR_PYTHON_NATIVE_RUNTIME_PREFIXES)
        list(APPEND runtime_rpaths "${origin}/${package_relative_base}${package_relative_prefix}/${CMAKE_INSTALL_LIBDIR}")
    endforeach()

    set("${output_variable}" "${runtime_rpaths}" PARENT_SCOPE)
endfunction()

function(tyr_make_python_native_runtime_rpath_string output_variable origin package_relative_base)
    tyr_make_python_native_runtime_rpaths(runtime_rpaths "${origin}" "${package_relative_base}")
    list(JOIN runtime_rpaths ":" runtime_rpath)
    set("${output_variable}" "${runtime_rpath}" PARENT_SCOPE)
endfunction()
