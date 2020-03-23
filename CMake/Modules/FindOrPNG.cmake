message("Warning: you are using FindOrPNG wich is marked ad depreciated!")

if(NOT OrPNG_FOUND AND NOT PNG_DIR AND NOT ZLIB_DIR)
  set(PNG_DIR "/usr/local/png/" CACHE PATH "Root of PNG tree (optional).")
  set(ZLIB_DIR "/usr/local/zlib/" CACHE PATH "Root of ZLIB tree (optional).")
endif()

find_library(ZLIB_LIBRARY zlib PATHS "${ZLIB_DIR}/Release" "${ZLIB_DIR}/lib")
find_library(ZLIB_LIBRARY_DEBUG zlibd PATHS "${ZLIB_DIR}/Debug" "${ZLIB_DIR}/lib")

set(PNG_NAMES libpng15 png15)
find_library(PNG_LIBRARY ${PNG_NAMES} PATHS "${PNG_DIR}/Release" "${PNG_DIR}/lib")
set(PNG_NAMES libpng15d png15d)
find_library(PNG_LIBRARY_DEBUG ${PNG_NAMES} PATHS "${PNG_DIR}/Debug" "${PNG_DIR}/lib")

mark_as_advanced(CLEAR ZLIB_LIBRARY ZLIB_LIBRARY_DEBUG PNG_LIBRARY PNG_LIBRARY_DEBUG)

if(PNG_LIBRARY)
  list(APPEND PNG_LIBRARIES optimized ${PNG_LIBRARY})
endif()
if(PNG_LIBRARY_DEBUG)
  list(APPEND PNG_LIBRARIES debug ${PNG_LIBRARY_DEBUG})
endif()

if(ZLIB_LIBRARY)
  list(APPEND PNG_LIBRARIES optimized ${ZLIB_LIBRARY})
endif()
if(ZLIB_LIBRARY_DEBUG)
  list(APPEND PNG_LIBRARIES debug ${ZLIB_LIBRARY_DEBUG})
endif()


find_path(ZLIB_INCLUDE_DIR zlib.h PATHS ${ZLIB_DIR})
find_path(PNG_INCLUDE_DIR png.h PATHS ${PNG_DIR})

mark_as_advanced(CLEAR ZLIB_INCLUDE_DIR PNG_INCLUDE_DIR)

if(ZLIB_INCLUDE_DIR)
  list(APPEND PNG_INCLUDE_DIRS ${ZLIB_INCLUDE_DIR})
endif()
if(PNG_INCLUDE_DIR)
  list(APPEND PNG_INCLUDE_DIRS ${PNG_INCLUDE_DIR})
endif()

#include(FindPackageHandleStandardArgs)
#find_package_handle_standard_args(OrPNG REQUIRED_VARS ZLIB_INCLUDE_DIR ZLIB_LIBRARY ZLIB_LIBRARY_DEBUG PNG_INCLUDE_DIR PNG_LIBRARY PNG_LIBRARY_DEBUG)
									   
set(OrPNG_FOUND FALSE)
if(PNG_LIBRARY AND ZLIB_LIBRARY)
   set(OrPNG_FOUND TRUE)
endif()
