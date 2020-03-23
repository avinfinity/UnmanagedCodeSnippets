#****************************************************************************
# (c) 2008 by the openOR Team
#****************************************************************************
# The contents of this file are available under the GPL v2.0 license
# or under the openOR comercial license. see 
#   /Doc/openOR_free_license.txt or
#   /Doc/openOR_comercial_license.txt 
# for Details.
#****************************************************************************

#
#  Special macros for convenient and transparent processing of Qt ui-files. 
#  Greatly inspired by the KDE3Macros.cmake module.
#
include(AddFileDependencies) # TODO: what is this good for?


macro(OR_SET_POLICY param_policy value)
   if(POLICY ${param_policy})
      CMAKE_POLICY(SET ${param_policy} ${value})
   endif()
endmacro()


#
# Creates and installes a new executable 
#
macro(OR_ADD_EXECUTABLE NAME)
   message(STATUS "creating Executable: ${NAME}") 
   or_handle_source_list(${ARGN})
   add_executable(${NAME} ${_PARAM1} ${_PARAM2} ${_PARAM3} ${_source_files})
   set(${NAME}_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "Include dir of ${NAME}")
   include_directories(BEFORE ${${NAME}_INCLUDE_DIR})
   OR_ADD_PATH_TO_ALL_INCLUDE_PATHS(${${NAME}_INCLUDE_DIR})
   set_target_properties(${NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

   or_install_interface_files(${NAME} ${_source_files})
   or_install_target_artefacts(${NAME})

   export(TARGETS ${NAME} FILE ${NAME}-exports.cmake)
   file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${NAME}-exports.cmake "\n" )
   file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${NAME}-exports.cmake "set(${NAME}_INCLUDE_DIR \"${CMAKE_CURRENT_SOURCE_DIR}/include\" CACHE INTERNAL \"Include dir of ${NAME}\") \n" )
   
   file(APPEND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake "include(\"${CMAKE_CURRENT_BINARY_DIR}/${NAME}-exports.cmake\")\n")
   
   or_disable_warnings()
endmacro()

#
# Creates and installes a new plugin or library  
#
macro(OR_ADD_LIBRARY NAME)
   or_handle_source_list(${ARGN})

   if (OPENOR_BUILD_STATIC)
      if (${_PARAM1} STREQUAL "SHARED")
         set(_PARAM1 "STATIC")
      endif()
   endif()

   if (${_PARAM1} STREQUAL "FORCE_SHARED")
      set(_PARAM1 "SHARED")
   endif()

   if (${_PARAM1} STREQUAL "MODULE")
      message(STATUS "creating Plugin: ${NAME}")
   else()
      message(STATUS "creating ${_PARAM1} Library: ${NAME}")
   endif()

   add_library(${NAME} ${_PARAM1} ${_PARAM2} ${_PARAM3} ${_source_files})
   set(${NAME}_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "Include dir of ${NAME}")
   include_directories(BEFORE ${${NAME}_INCLUDE_DIR})
   OR_ADD_PATH_TO_ALL_INCLUDE_PATHS(${${NAME}_INCLUDE_DIR})
   set(${NAME}_LIBRARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR} CACHE INTERNAL "Library dir of ${NAME}")
   link_directories(${${NAME}_LIBRARY_DIR})  
   set (${NAME}_LIBRARIES debug ${NAME}${CMAKE_DEBUG_POSTFIX} optimized ${NAME} CACHE INTERNAL "Link Libraries of ${NAME}")   

   # TODO: fix this to be set for all dependent targets (move to dependency funct?)
   #if (${_PARAM1} STREQUAL "STATIC")
   #   ADD_DEFINITIONS("-D${NAME}_STATIC")
   #endif()

   or_install_interface_files(${NAME} ${_source_files})
   or_install_target_artefacts(${NAME})
   
   export(TARGETS ${NAME} FILE ${NAME}-exports.cmake)
   file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${NAME}-exports.cmake "\n" )
   file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${NAME}-exports.cmake "set(${NAME}_INCLUDE_DIR \"${CMAKE_CURRENT_SOURCE_DIR}/include\" CACHE INTERNAL \"Include dir of ${NAME}\") \n" )
   file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${NAME}-exports.cmake "set(${NAME}_LIBRARY_DIR \"${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}\" CACHE INTERNAL \"Library dir of ${NAME}\") \n" )
   file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${NAME}-exports.cmake "set(${NAME}_LIBRARIES debug ${NAME}${CMAKE_DEBUG_POSTFIX} optimized ${NAME} CACHE INTERNAL \"Link Libraries of ${NAME}\") \n")  

   file(APPEND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake "include(\"${CMAKE_CURRENT_BINARY_DIR}/${NAME}-exports.cmake\")\n")
   
   if(MSVC)
      # Force to always compile with W4
      if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
         string(REGEX REPLACE "/W[0-4]" "/wd4996 /wd4251 /wd4275" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
      else()
         set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996 /wd4251 /wd4275")
      endif()
   elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
      # Update if necessary
   #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
   endif()
   
   or_disable_warnings()
endmacro()

#
# Creates and installes a new plugin interface
#
macro(OR_ADD_INTERFACE NAME MODULE)
   message(STATUS "creating Interface: ${NAME}")
   
   # TODO check if all commands are realy needed
   #add_library(${NAME} ${ARGN})
   add_custom_target(${NAME} SOURCES ${ARGN})
   set(${NAME}_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "Include dir of ${NAME}")
   include_directories(BEFORE ${${NAME}_INCLUDE_DIR})
   OR_ADD_PATH_TO_ALL_INCLUDE_PATHS(${${NAME}_INCLUDE_DIR})
   
   or_install_interface_files(${NAME} ${_source_files})
   #or_install_target_artefacts(${NAME})
   SET_TARGET_PROPERTIES(${NAME} PROPERTIES LINKER_LANGUAGE CXX)

   #export(TARGETS ${NAME} FILE ${NAME}-exports.cmake)
   file(WRITE  ${CMAKE_CURRENT_BINARY_DIR}/${NAME}-exports.cmake "# Auto generated export file\n" )
   file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${NAME}-exports.cmake "\n" )
   file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${NAME}-exports.cmake "set(${NAME}_INCLUDE_DIR \"${CMAKE_CURRENT_SOURCE_DIR}/include\" CACHE INTERNAL \"Include dir of ${NAME}\") \n" )
   
   file(APPEND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake "include(\"${CMAKE_CURRENT_BINARY_DIR}/${NAME}-exports.cmake\")\n")
   
   or_disable_warnings()
endmacro()


#
# Installs non code Resource files
#
macro(OR_ADD_RESOURCE NAME REL_PATH)
   # TODO check if this is correct on all platforms
   OR_COPY_IF_DIFFERENT(${NAME} "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/${REL_PATH}" ${ARGN} )
 
   if (${openOR_${NAME}_NO_INSTALL})
      #message(STATUS "    Resources for '${NAME}' will not be installed.")
   else()
      install(FILES ${ARGN} DESTINATION "bin/${REL_PATH}")
   endif()
       
endmacro()

#
#creates a dictionary for files that differ a lot, e.g. boost*.lib's and boost*.dll's
#scheme "original filename" "revised filename"
#note: also contains optimized and debug but twice if there are part of input list
#note: macro is at the moment only making changes on file names for boost libraries, when required for other things it needs to be adjusted
#
macro(OR_CREATE_DICTIONARY LIBRARY_LIST)
	set(tempList "")
	if(${LIBRARY_LIST} MATCHES "Boost")												#so other additions to the dictionary can be made
		foreach(arg ${ARGV})
			list(APPEND tempList ${arg})											#put the original file name before the revised file name
			string(REGEX REPLACE "libboost" "boost" temp1 ${arg})					#transform libboost to boost
			string(REGEX REPLACE "-sgd" "-gd" temp2 ${temp1})						#delete the -s from debug library name
			string(REGEX REPLACE "-s-" "-" temp3 ${temp2})							#delete the -s from release library name
			string(REGEX REPLACE  "[.]lib" ".dll" temp4 ${temp3})
			list(APPEND tempList ${temp4})
		endforeach(arg)
	endif(${LIBRARY_LIST} MATCHES "Boost")
	list(APPEND INTERNAL_DICTIONARY ${tempList})
endmacro(OR_CREATE_DICTIONARY)

#
# adds external library, links and copies it
#
macro(OR_ADD_EXTERNAL_LIBRARY TARGET)
	set(DEFAULT_LIBRARY_SEARCH_PATHS "/usr/local/lib/" "/usr/lib/" $ENV{PATH})		#default paths to be searched
	#first declare the library extension for the current os
	if(WIN32)																		#if WIN32 os
		set(SYS_EXT ".dll")
	elseif(APPLE)																	#if APPLE os
		set(SYS_EXT ".dylib")
	else(WIN32)																		#if other unix, e.g. linux
		set(SYS_EXT ".so")
	endif(WIN32)
	#variables used:
	#arg - a single argument from ARGV
	#the following arguments are only used if arg is an existing file
	#index - index at which a filename can be found if it is part of the entries in the dictionary
	#new_index - one position further; where the revised entry in the dictionary can be found
	#src_ext - extension of an argument
	#src_dir - directory of an argument
	#src_filename - name of an argument with the extension used for libraries in current os
	#src_ch - src changed, filename with revised extension
	foreach(arg ${ARGV})															#search the libraries for directories and/or files to copy
		if(EXISTS ${arg} AND NOT(IS_DIRECTORY ${arg}))								#if current arg is a file
			List(FIND INTERNAL_DICTIONARY "${arg}" index)							#try to find the arg in the dictionary
			if(${index} GREATER -1)													#if the arg is found
				math(EXPR new_index ${index}+1)
				List(GET INTERNAL_DICTIONARY ${new_index} arg)						#get the needed name from the dictionary
			endif(${index} GREATER -1)
			get_filename_component(src_ext ${arg} EXT)								#get the extension of the file
			get_filename_component(src_dir ${arg} PATH)								#get the path of the file and copy the directory
			get_filename_component(src_filename ${arg} NAME)						#get the filename including extension
			set(output "${arg}")
			if(WIN32)
				OR_CHECK_FOR_WIN_SYSTEM_FOLDERS(${output})
			endif(WIN32)
			if(NOT ((${sys32_folder} MATCHES "C:/Windows/System32") OR (${sys_folder} MATCHES "C:/Windows/system")))
				or_internal_copy_ext_lib(${TARGET} "${output}" "${EXTERNAL_LIBRARY_DESTINATION}/" "${src_filename}")
			endif(NOT ((${sys32_folder} MATCHES "C:/Windows/System32") OR (${sys_folder} MATCHES "C:/Windows/system")))
		else(EXISTS ${arg} AND NOT(IS_DIRECTORY ${arg}))																#some libraries may not be specified with their file names
			set(output "output-NOTFOUND")
			find_file(output ${arg} ${EXTERNAL_LIBRARY_PATHS} "${DEFAULT_LIBRARY_SEARCH_PATHS}" "${src_dir}/../bin/" "${src_dir}/../lib/" "${src_dir}" "${src_dir}/lib/" "${src_dir}/bin/")
			if(NOT(IS_DIRECTORY ${output}))											#if the library is found and not a directory
				get_filename_component(src_ext ${output} EXT)						#get the extension of the file
				get_filename_component(src_dir ${output} PATH)						#get the path of the file and copy the directory
				get_filename_component(src_filename ${output} NAME)					#get the filename including extension
				if(NOT (${output} MATCHES "output-NOTFOUND"))
					if(WIN32)
						OR_CHECK_FOR_WIN_SYSTEM_FOLDERS(${output})
					endif(WIN32)
					if(NOT ((${sys32_folder} MATCHES "C:/Windows/System32") OR (${sys_folder} MATCHES "C:/Windows/system")))
						or_internal_copy_ext_lib(${TARGET} "${output}" "${EXTERNAL_LIBRARY_DESTINATION}/" "${src_filename}")
					endif(NOT ((${sys32_folder} MATCHES "C:/Windows/System32") OR (${sys_folder} MATCHES "C:/Windows/system")))
				endif(NOT (${output} MATCHES "output-NOTFOUND"))
			endif(NOT(IS_DIRECTORY ${output}))
		endif(EXISTS ${arg} AND NOT(IS_DIRECTORY ${arg}))
	endforeach(arg)
 	target_link_libraries(${ARGV})
endmacro(OR_ADD_EXTERNAL_LIBRARY())


#
#generates strings from input to check them for being the system folders of windows which then don't need to be copied
#
macro(OR_CHECK_FOR_WIN_SYSTEM_FOLDERS INPUT)
	set(sys32_folder "")
	set(sys_Folder "")
	string(SUBSTRING ${INPUT} 0 19 sys32_folder)
	string(SUBSTRING ${INPUT} 0 17 sys_folder)
endmacro()

#
# Add a Library dependency to an existing target.
# TODO: needs a proper name like add_lib_dependency
#


macro(OR_ADD_DEPENDENCIES TARGET)
	or_specify_dependencies(${TARGET} ${ARGN})
	or_execute_dependencies(${TARGET})
endmacro()

#
# Add a Plugin or Interface dependency to an existing target.
# TODO: needs a proper name like add_plugin_dependencies
#
macro(OR_ADD_INCLUDES TARGET)
	#add_dependencies(${TARGET} ${ARGN}) # TODO fix the circular dependency in MouseHdl/CameraConfig
    foreach(_dependency ${ARGN})
		include_directories(BEFORE SYSTEM ${${_dependency}_INCLUDE_DIR})
		copy_if_imported(${TARGET} ${_dependency})
	endforeach(_dependency)
endmacro()

macro(OR_ADD_RUNTIME_DEPENDENCIES TARGET)
	add_dependencies(${TARGET} ${ARGN}) 
   foreach(_dependency ${ARGN})
      copy_if_imported(${TARGET} ${_dependency})
	endforeach(_dependency)
endmacro()

macro (copy_if_imported TARGET DEP) 
   
   #todo: this doesn't work, find out how it is supposed to work.
   if(CMAKE_BUILD_TYPE)
      string(TOUPPER ${CMAKE_BUILD_TYPE} _build_type)
      set(_import "IMPORTED_LOCATION_${_build_type}")
   else()
      set(_import "IMPORTED_LOCATION")
   endif()
   
   get_target_property(_dll ${DEP} ${_import})
   if (_dll)
      ADD_CUSTOM_COMMAND(
         TARGET ${TARGET} PRE_BUILD
         COMMAND ${CMAKE_COMMAND}
         ARGS    -E copy "${_dll}" "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/"
         COMMENT "Copying ${_dll} to ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/"
      )
   endif()
endmacro()



macro(OR_ADD_PATH_TO_ALL_INCLUDE_PATHS PATH)
   set(OPENOR_ALL_INCLUDE_PATHS ${OPENOR_ALL_INCLUDE_PATHS} CACHE INTERNAL "All Include directories")
   list(APPEND OPENOR_ALL_INCLUDE_PATHS ${ARGV0})
   list(REMOVE_DUPLICATES OPENOR_ALL_INCLUDE_PATHS)
   set(OPENOR_ALL_INCLUDE_PATHS ${OPENOR_ALL_INCLUDE_PATHS} CACHE INTERNAL "All Include directories")
   set(OPENOR_ALL_INCLUDE_PATHS_DOXYGENSTRING "" CACHE INTERNAL "All Include directories doxygen string")
   foreach(path_  ${OPENOR_ALL_INCLUDE_PATHS})
      set(OPENOR_ALL_INCLUDE_PATHS_DOXYGENSTRING "${OPENOR_ALL_INCLUDE_PATHS_DOXYGENSTRING} \"${path_}\"" CACHE INTERNAL "All Include directories doxygen string")
   endforeach(path_)
endmacro()




#
# 
#
macro(create_openOR_package_info)
   file(WRITE  ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake "# CMake Package Config for ${PROJECT_NAME}, created automatically, do not modify!\n\n")
   file(APPEND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake "set(${PROJECT_NAME}_FOUND true)\n")
   file(APPEND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake "get_filename_component(${PROJECT_NAME}_ROOT_DIR \${CMAKE_CURRENT_LIST_FILE} PATH)\n")
   file(APPEND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake "set(${PROJECT_NAME}_SOURCE_DIR \"${CMAKE_SOURCE_DIR}\")\n")
   file(APPEND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake "include(\"${CMAKE_MODULE_PATH}/openORMacros.cmake\")\n")
   
   
   set(CMAKE_DEBUG_POSTFIX ${openOR_DEBUG_SUFFIX})
   
   file(APPEND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake "\n")
endmacro()

macro(add_external_includes_to_openOR_package_info)
   get_directory_property(_inc_dir INCLUDE_DIRECTORIES)
   get_directory_property(_link_dir LINK_DIRECTORIES)
   
   openOR_detail_escape_list(_inc_dir)
   openOR_detail_escape_list(_link_dir)

   file(APPEND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake "include_directories(${_inc_dir})\n")
   file(APPEND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake "link_directories   (${_link_dir})\n")
   file(APPEND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake "set(CMAKE_DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})\n")
   
   # TODO: these should not be here, but be somehow handled automatically
   file(APPEND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake "set(QT_LIBRARIES           ${QT_LIBRARIES})\n")
   file(APPEND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake "set(OPENGL_LIBRARIES       ${OPENGL_LIBRARIES})\n")
   file(APPEND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake "set(EXECUTABLE_OUTPUT_PATH \"\${PROJECT_BINARY_DIR}/bin\")\n")
   file(APPEND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake "set(LIBRARY_OUTPUT_PATH    \"\${PROJECT_BINARY_DIR}/bin\")\n")
   
   file(APPEND ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-config.cmake "\n")
endmacro()


#-------------------------------------------------------------------------------------------------------------------------------
# Test Suite
#-------------------------------------------------------------------------------------------------------------------------------


macro(OR_ADD_TEST_TARGET NAME)
   or_handle_source_list(${ARGN})
   
   add_executable(${NAME} ${_PARAM1} ${_PARAM2} ${_PARAM3} ${_source_files})
   set_target_properties(${NAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
endmacro()

#
#  This macro depends on ${TEST_PROJECT}  and ${TEST_BINARY_DIR}  to be set. 
#
#  This macro is intended to be used in the project to be tested, not in the test-project! or_add_test_target will
#  be used in test-projects.  
#
# TODO: this doesn't work properly.
#
macro(or_add_test TARGET TEST_PROJECT TEST_TARGET)
   
   add_test(${TEST_TARGET} ${CMAKE_CTEST_COMMAND} 
      --build-and-test     ${CMAKE_SOURCE_DIR} ${TEST_BINARY_DIR}
      --build-noclean
      --build-project      ${TEST_PROJECT}
      --build-target       ${TEST_TARGET}
	  --build-generator ${CMAKE_GENERATOR}
	  --build-makeprogram ${CMAKE_BUILD_TOOL}
   )
   
   or_disable_warnings()
   
   # Do not run the tests as a POST_BUILD command for these reasons:
   # 1. This increases the build times too much!
   # 2. Building of components-under-test fails, if the tests fail. This is bad, if the interface-files of this
   #    component first need to be installed before the tests can be built!
   # 3. For some reason the output of cmake building the test-solution (as a post-build step) on VS2005 is way too much. 
   #    This is quite irreproducible and confusing for users of the unit-tests.
   #add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${_bt_makeprogram} test WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

endmacro()

#-------------------------------------------------------------------------------------------------------------------------------
# IMPLEMENTATION DETAILS 
# the macros below are not supposed to be used directly
#-------------------------------------------------------------------------------------------------------------------------------

#
#  This macro adds something to a target property.
#
macro(OR_ADD_TO_TARGET_PROPERTY TARGET PROPERTY)
   set(_values "")
   foreach(_prop_value ${ARGN})
      set(_values "${_values} ${_prop_value}")
   endforeach(_prop_value)

   get_target_property(_OLD_PROP ${TARGET} ${PROPERTY})
   if(NOT _OLD_PROP)
      set(_OLD_PROP "")
   endif(NOT _OLD_PROP)
   set_target_properties(${TARGET} PROPERTIES ${PROPERTY} "${_OLD_PROP} ${_values}")
   set(_OLD_PROP "")
endmacro(OR_ADD_TO_TARGET_PROPERTY)


macro(OR_ADD_LINKS TARGET)
	foreach(_dependency ${ARGN})
      link_directories(${${_dependency}_LIBRARY_DIR})
      target_link_libraries(${TARGET} ${${_dependency}_LIBRARIES})
      get_target_property(_ttype ${TARGET} TYPE)
      if (_ttype STREQUAL "MODULE_LIBRARY")
         set_target_properties(${TARGET} PROPERTIES LINK_INTERFACE_LIBRARIES "")
         set_target_properties(${TARGET} PROPERTIES LINK_INTERFACE_LIBRARIES_DEBUG "")
         set_target_properties(${TARGET} PROPERTIES LINK_INTERFACE_LIBRARIES_RELEASE "")
      endif()
	endforeach(_dependency)
endmacro(OR_ADD_LINKS)


macro(openOR_detail_escape_list LISTVAR)
   set(_result_list "")
   set(_result "")
   list(REMOVE_DUPLICATES ${LISTVAR})
   list(LENGTH ${LISTVAR} _list_size)
   math(EXPR _list_size ${_list_size}-1)
   foreach(_list_it RANGE ${_list_size})
      list(GET ${LISTVAR} ${_list_it} _current)
      list(APPEND _result_list \"${_current}\")
      set(_result "${_result} \"${_current}\"")
   endforeach()
   #set(${LISTVAR} ${_result_list})
   set(${LISTVAR} ${_result})
 endmacro()


MACRO(OR_COPY_IF_DIFFERENT TARGETNAME TO_DIR FILES)
   SET(_files ${ARGV})
   LIST(REMOVE_AT _files 0 1)
   
   foreach(FILE ${_files})
      SET(SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${FILE})
      GET_FILENAME_COMPONENT(FILENAME ${FILE} NAME)
      SET(DESTINATION_FILE ${TO_DIR}/${FILENAME})
      ADD_CUSTOM_COMMAND(
         TARGET ${TARGETNAME} PRE_BUILD
         COMMAND ${CMAKE_COMMAND}
         ARGS    -E copy_if_different ${SOURCE_FILE} ${DESTINATION_FILE}
         COMMENT "Copying ${SOURCE_FILE} to ${DESTINATION_FILE}"
      )
      #message("Set Copying ${FILE} ${SOURCE_FILE} to ${DESTINATION_FILE}")
   endforeach(FILE)
ENDMACRO(OR_COPY_IF_DIFFERENT TARGETNAME TO_DIR FILES)

#
#  Only to be used internally by another macro!
#
macro(OR_HANDLE_SOURCE_LIST)
   # Initialize used variables to "empty"
   set(_source_files "")
   set(_ui_files "")
   set(_PARAM1 "")
   set(_PARAM2 "")
   set(_PARAM3 "")
   
   if(NOT ${ARGV0} STREQUAL "")
      string(REGEX MATCH WIN32|SHARED|FORCE_SHARED|STATIC|MODULE _param1_match ${ARGV0})
      set(_PARAM1 ${_param1_match})
   endif(NOT ${ARGV0} STREQUAL "")
   if(NOT ${ARGV1} STREQUAL "")
      string(REGEX MATCH MACOSX_BUNDLE|EXCLUDE_FROM_ALL _param2_match ${ARGV1})
      set(_PARAM2 ${_param2_match})
   endif(NOT ${ARGV1} STREQUAL "")
   if(NOT ${ARGV2} STREQUAL "")
      string(REGEX MATCH EXCLUDE_FROM_ALL _param3_match ${ARGV2})
      set(_PARAM3 ${_param3_match})
   endif(NOT ${ARGV2} STREQUAL "")

   # Iterate over the given list of source files to be included into the executable. This list will be eventually 
   # expanded by (e.g. moc-) files which should not be considered in the iteration => Using ARGN instead of ARGV!
   foreach(_current_file ${ARGV})
      # Collect source code files
      string(REGEX MATCH \\.cpp$|\\.c$|\\.cc$|\\.h$|\\.hpp$|\\.css$|\\.xml$ source_file_match ${_current_file})
      if(source_file_match)
         list(APPEND _source_files ${_current_file})
      endif(source_file_match)
      
      # Collect ui-files
      string(REGEX MATCH \\.ui$ ui_file_match ${_current_file})
      if(ui_file_match)
         list(APPEND _ui_files ${_current_file})
      endif(ui_file_match)
   endforeach(_current_file)
   
   # Check for Q_OBJECTS to be automoc'ed 
   if(_source_files)
      or_automoc(${_source_files})
   endif(_source_files)
   
   # Run Qt's ui-compiler on the ui-files and automoc the generated headers
   if(_ui_files)
      foreach(_ui_file ${_ui_files})
         or_autouic(${_ui_file} ${_source_files})
         list(APPEND _source_files ${_ui_file})    # Add ui-file to target, to make it accessible e.g. via the VS8 GUI
      endforeach(_ui_file)
   endif(_ui_files)
   
endmacro(OR_HANDLE_SOURCE_LIST)

#
#  Only to be used internally by another macro!
#
macro(OR_AUTOMOC _source_files)

   foreach(_source_file ${ARGV})
      get_filename_component(_abs_file ${_source_file} ABSOLUTE)
      get_filename_component(_abs_path ${_abs_file} PATH)
      
      if (EXISTS ${_abs_file})      
         file(READ ${_abs_file} _contents)
         string(REGEX MATCH "[ ]*Q_OBJECT[;| ]*" _match "${_contents}")
         if(_match)
            
            #message("'${_abs_file}' must be auto-moced...")
         
            # TODO: 1) Check, if _abs_file is a header and has a corresponding cpp-file.
            #       2) If 1) == yes, check if cpp-file includes a .moc-file. 
            #       3) If 2) == yes, name the moc to be created accordingly and do not add it to _source_files
            
            get_filename_component(_basename ${_source_file} NAME_WE)
            get_filename_component(_ext ${_source_file} EXT)
            set(_header ${_abs_path}/${_basename}${_ext})
            set(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc.cpp)

            add_custom_command(OUTPUT ${_moc}
               COMMAND ${QT_MOC_EXECUTABLE}
               ARGS ${_header} -o ${_moc}
               DEPENDS ${_header}
            )
            add_file_dependencies(${_abs_FILE} ${_moc})
            list(APPEND _source_files ${_moc})
         endif(_match)      
      endif (EXISTS ${_abs_file})
   endforeach(_source_file)
endmacro(OR_AUTOMOC)

#
#  Only to be used internally by another macro!
#
macro(OR_AUTOUIC ui_file)
         
   get_filename_component(_tmp_file ${ui_file} ABSOLUTE)
   get_filename_component(_basename ${_tmp_file} NAME_WE)
   set(_header ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.ui.h)
   set(_src ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.ui.cpp)
   set(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc.cpp)
   
   add_custom_command(OUTPUT ${_header}
      COMMAND ${QT_UIC_EXECUTABLE}
      ARGS -o ${_header} ${CMAKE_CURRENT_SOURCE_DIR}/${ui_file}
      DEPENDS ${_tmp_file}
   )

   add_custom_command(OUTPUT ${_src}
      COMMAND ${QT_UIC_EXECUTABLE}
      ARGS -i ${_header} -o ${_src} ${CMAKE_CURRENT_SOURCE_DIR}/${ui_file}
      DEPENDS ${_header}
   )

   add_custom_command(OUTPUT ${_moc}
      COMMAND ${QT_MOC_EXECUTABLE}
      ARGS ${_header} -o ${_moc}
      DEPENDS ${_header}
   )
   # _source_files has to be defined in the calling macro!
   list(APPEND _source_files ${_src} ${_moc})
   
endmacro(OR_AUTOUIC)

#
#  * sets variable "Interface_Files" to the interface-files contained in the given list of files
#  * sets variable "_include_subdir" to the relative dir containing the openOR interface-files
#
macro(OR_PREPARE_INTERFACE_FILES)
   set(Interface_Files "")
   set(_include_subdir include)
   foreach(file ${ARGN})
   	string(REGEX MATCH ^include/.+ INTERFACE_MATCH_1 ${file})
   	string(REGEX MATCH .*/include/.+ INTERFACE_MATCH_2 ${file})
   	set(_is_interface_file FALSE)
   	if(INTERFACE_MATCH_1 STREQUAL ${file})
         set(_is_interface_file TRUE)
      endif(INTERFACE_MATCH_1 STREQUAL ${file})
      if(INTERFACE_MATCH_2 STREQUAL ${file})
         set(_is_interface_file TRUE)
      endif(INTERFACE_MATCH_2 STREQUAL ${file})
   	
   	if(_is_interface_file)
      	list(APPEND Interface_Files ${file})
   	endif(_is_interface_file)
   endforeach(file)
endmacro(OR_PREPARE_INTERFACE_FILES)

#
#  This adds a rule to install the interface files to the "include"-subdir of ${CMAKE_INSTALL_PREFIX}.
#
#  Note: We could have used install(DIRECTORIES ...) to install each component's interface-directory. But
#        we have abandoned this approach. It would have made it possible to install interface-files that are
#        not part of the actual target, i.e. files that are no longer in use. There would have been a discrepancy
#        between the files defined as "part-of-the-target" in the <targetname>_FILES-variable and the files that are
#        actually beeing installed! 
#
macro(OR_INSTALL_INTERFACE_FILES TargetName)
   or_prepare_interface_files(${ARGN})
   
   foreach(_file ${Interface_Files})
      string(REGEX REPLACE "^include/|.*/include/" "" _subdir ${_file})
      get_filename_component(_subdir ${_subdir} PATH)
      install(FILES ${_file} DESTINATION ${_include_subdir}/${_subdir} ${Optional_Installs})
   endforeach(_file)
      
endmacro(OR_INSTALL_INTERFACE_FILES)

#
#  Macro to install openOR targets
#
macro(OR_INSTALL_TARGET_ARTEFACTS TargetName)
   
   #get_target_property(_is_plugin ${TargetName} IS_PLUGIN)
   set(_subdir "")
   #if(_is_plugin)
   #   set(_subdir "/plugins")
   #endif(_is_plugin)

   # using if(NOT ${openOR...} does not work correctly.
   if (${openOR_${TargetName}_NO_INSTALL})
      message(STATUS "    Target '${TargetName}' will not be installed.")
   else()

      #install(TARGETS ${TargetName} EXPORT ${TargetName}-targets
      install(TARGETS ${TargetName}
         RUNTIME DESTINATION bin${_subdir} ${Optional_Installs}
         LIBRARY DESTINATION bin${_subdir} ${Optional_Installs}
         ARCHIVE DESTINATION lib${_subdir} ${Optional_Installs}
      )
      #install(EXPORT ${TargetName}-targets DESTINATION lib)
      #message(STATUS "    Target '${TargetName}' added to install set.")


   endif()

endmacro(OR_INSTALL_TARGET_ARTEFACTS)


macro(or_internal_copy_ext_lib TARGET SRC DEST LIB)

   #message(STATUS "Copying ${output} to ${EXTERNAL_LIBRARY_DESTINATION}")
	ADD_CUSTOM_COMMAND(	COMMAND ${CMAKE_COMMAND} 
	                    ARGS    -E copy_if_different "${SRC}" "${DEST}/${LIB}"
						TARGET ${TARGET}
						COMMENT "Copying required libraries for ${TARGET} to ${DEST}"
	)
   
   if (${openOR_${TARGET}_NO_INSTALL})
      #message(STATUS "    External Libs for '${TARGET}' will not be installed.")
   else()
      # workaround to handle MacOS X frameworks/bundles
      if (IS_DIRECTORY "${SRC}")
         #install(DIRECTORY ${SRC} DESTINATION "${DEST}/")
         #  ^ this does not seem to work with frameworks.
      else()
         install(FILES ${SRC} DESTINATION "${DEST}/")
      endif()
   endif()
   
endmacro()


macro(or_project_group PRJCT FLDR)

	# group project to a folder-path
	set_property(TARGET ${PRJCT} PROPERTY FOLDER ${FLDR})
	
	# separate folders
	
	# adopt folder names
	#STRING(REPLACE " " "" "_fldr" ${FLDR})
	#STRING(REPLACE "/" "_" "_fldr" ${_fldr})
	
	# store project in every parental folder lists
	#if (NOT DEFINED _ProjectGroup_${_fldr})
	#	set(_ProjectGroup_${_fldr} "" CACHE STRING "....")
	#	message("new group")
	#endif()
	
	#set(_ProjectGroup_${_fldr} "${_ProjectGroup_${_fldr}};${PRJCT}" CACHE STRING "...")
	#list(APPEND _ProjectGroup_${_fldr} ${PRJCT})
	#list(LENGTH _ProjectGroup_${_fldr} "_cnt")
	
	#message(${_cnt})
	#message(${PRJCT})
   
endmacro()

macro(or_add_group_dependencies FLDR)
	
	foreach(_folder ${_projects_by_groups})
		
	endforeach(_file)
	
endmacro()

# Transitive dependency resolution

macro(or_specify_dependencies TARGET)
	#message("In ${ARGN}")
   
   list(APPEND _TRANSITIVE_RESOLVER_FOR_${TARGET} ${ARGN})
   list(REMOVE_DUPLICATES _TRANSITIVE_RESOLVER_FOR_${TARGET})
   
   set(_TRANSITIVE_RESOLVER_FOR_${TARGET} ${_TRANSITIVE_RESOLVER_FOR_${TARGET}} CACHE INTERNAL "")
   set(_TRANSITIVE_DEPS_${TARGET} ${_TRANSITIVE_DEPS_${TARGET}} CACHE INTERNAL "")
   
	#message("After ${_TRANSITIVE_RESOLVER_FOR_${TARGET}}")
endmacro(or_specify_dependencies)

macro(or_collect_transitive_deps TARGET DEP)
	#message("${TARGET} depends on ${ARGN} over ${DEP}")
	list(APPEND _TRANSITIVE_DEPS_${TARGET} ${ARGN})
	list(REMOVE_DUPLICATES _TRANSITIVE_DEPS_${TARGET})
    foreach(_dependency ${_TRANSITIVE_RESOLVER_FOR_${DEP}})
		list(LENGTH _TRANSITIVE_RESOLVER_FOR_${_dependency} _dep_len)
		if (_dep_len)
			#message("Adding deps of ${_dependency} to ${TARGET} (${_TRANSITIVE_RESOLVER_FOR_${_dependency}})")
			or_collect_transitive_deps(${TARGET} ${_dependency} ${_TRANSITIVE_RESOLVER_FOR_${_dependency}})
		endif ()
	endforeach(_dependency)
endmacro()

macro(or_execute_transitive_deps TARGET)
	#message("Adding transitive deps to ${TARGET} (${ARGN})")
    add_dependencies(${TARGET} ${ARGN})
    or_add_includes(${TARGET} ${ARGN})
    or_add_links(${TARGET} ${ARGN})
endmacro()

macro(or_execute_dependencies TARGET)
   or_collect_transitive_deps(${TARGET} ${TARGET} ${_TRANSITIVE_RESOLVER_FOR_${TARGET}})
   or_execute_transitive_deps(${TARGET} ${_TRANSITIVE_DEPS_${TARGET}})
endmacro(or_execute_dependencies)

macro(or_disable_warnings)
   if(MSVC)
      # Force to always compile with W4
      if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
         string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
      endif()
      # disable warnings:
      # - C4251 and C4275: VS doesn't like the usage of STL or boost templates in exported classes.
      # - C4996: VS declares functions as deprecated, if it can offer a better alternative, e.g. fopen to fopen_s.
      #          This warning also appears within all OPENOR_CREATE_INTERFACE macros.
      # - C4217: "conditional expression is constant". This is the price of cross plattform coding: On some systems
      #          expressions like sizeof(ulong) > sizeof(uint) are constant. Mostly common in Qt files and templates.
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996 /wd4251 /wd4275 /wd4127")
      
   elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
      # Update if necessary
      #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
   endif()
endmacro(or_disable_warnings)


