#----------------------------------------------------------------
# Generated CMake target import file for configuration "DEBUG".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "llhttp::llhttp_shared" for configuration "DEBUG"
set_property(TARGET llhttp::llhttp_shared APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(llhttp::llhttp_shared PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/lib/libllhttp.dll.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS llhttp::llhttp_shared )
list(APPEND _IMPORT_CHECK_FILES_FOR_llhttp::llhttp_shared "${_IMPORT_PREFIX}/lib/libllhttp.dll.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
