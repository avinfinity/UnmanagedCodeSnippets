function(openOR_find_external)
   openOR_internal_dbgMessage("Finding Boost libraries")
   set(Boost_COMPONENTS filesystem system thread program_options serialization date_time)

   set(Boost_USE_MULTITHREADED ON)

   option(openOR_BOOST_STATIC_LIBS "Link against static boost libraries" ON)
   if(openOR_BOOST_STATIC_LIBS)
      set(Boost_USE_STATIC_LIBS ON)
   endif()

   option(openOR_BOOST_STATIC_RUNTIME "Tell boost to use static c++ stdlibs" OFF)
   if(openOR_BOOST_STATIC_RUNTIME)
      set(Boost_USE_STATIC_RUNTIME ON)
   endif()

   option(openOR_BOOST_DEBUG "Include debug symbols in boost libs" OFF)
   if(openOR_BOOST_DEBUG)
      set(Boost_DEBUG ON)
   endif()

    find_package(Boost COMPONENTS ${Boost_COMPONENTS})

   if(Boost_FOUND)
      set(openOR_internal_target_Boost_interfaces ${Boost_INCLUDE_DIRS} CACHE INTERNAL "Include dir of Boost")
      set(openOR_internal_target_Boost_binaries   ${Boost_LIBRARY_DIRS} CACHE INTERNAL "Binary dir of Boost")

      foreach(_comp ${Boost_COMPONENTS})
         openOR_internal_dbgMessage("Library file for ${_comp} = '${Boost_${_comp}_LIBRARY}'")
      endforeach()

      set(openOR_internal_target_Boost_link ${Boost_LIBRARIES} CACHE INTERNAL "What needs to be linked for target Boost")
      add_definitions(-DBOOST_ALL_NO_LIB)        # Disable Autolink feature
      if (NOT openOR_BOOST_STATIC_LIBS)
         add_definitions(-DBOOST_ALL_DYN_LINK)
      endif()

      set(Boost_FOUND ${Boost_FOUND} CACHE INTERNAL "We did find boost")
   endif()

endfunction()
