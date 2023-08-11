set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE static)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_DISABLE_COMPILER_TRACKING ON)

if (PORT EQUAL "tbb")
  set(VCPKG_LIBRARY_LINKAGE dynamic)
endif ()
