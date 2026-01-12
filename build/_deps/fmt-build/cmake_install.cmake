# Install script for directory: C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/cppcorn")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/Users/daniyal.hassan/scoop/apps/mingw/current/bin/objdump.exe")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-build/libfmtd.dll.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-build/bin/libfmtd.dll")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/libfmtd.dll" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/libfmtd.dll")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "C:/Users/daniyal.hassan/scoop/apps/mingw/current/bin/strip.exe" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/libfmtd.dll")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/fmt" TYPE FILE FILES
    "C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-src/include/fmt/args.h"
    "C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-src/include/fmt/chrono.h"
    "C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-src/include/fmt/color.h"
    "C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-src/include/fmt/compile.h"
    "C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-src/include/fmt/core.h"
    "C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-src/include/fmt/format.h"
    "C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-src/include/fmt/format-inl.h"
    "C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-src/include/fmt/os.h"
    "C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-src/include/fmt/ostream.h"
    "C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-src/include/fmt/printf.h"
    "C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-src/include/fmt/ranges.h"
    "C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-src/include/fmt/std.h"
    "C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-src/include/fmt/xchar.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt" TYPE FILE FILES
    "C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-build/fmt-config.cmake"
    "C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-build/fmt-config-version.cmake"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt/fmt-targets.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt/fmt-targets.cmake"
         "C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-build/CMakeFiles/Export/lib/cmake/fmt/fmt-targets.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt/fmt-targets-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt/fmt-targets.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt" TYPE FILE FILES "C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-build/CMakeFiles/Export/lib/cmake/fmt/fmt-targets.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt" TYPE FILE FILES "C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-build/CMakeFiles/Export/lib/cmake/fmt/fmt-targets-debug.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "C:/Users/daniyal.hassan/CLionProjects/cppcorn/build/_deps/fmt-build/fmt.pc")
endif()

