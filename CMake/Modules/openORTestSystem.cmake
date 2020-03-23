#****************************************************************************
# (c) 2012 by the openOR Team
#****************************************************************************
# The contents of this file are available under the GPL v2.0 license
# or under the openOR comercial license. see
#   /Doc/openOR_free_license.txt or
#   /Doc/openOR_comercial_license.txt
# for Details.
#****************************************************************************



#-------------------------------------------------------------------------------------------------------------------------------
# Test System
#-------------------------------------------------------------------------------------------------------------------------------

#
# add specific tests using the declarative testing language
# NOT READY FOR USAGE
macro(or_add_test declarative_expression)
	#not implemented yet
endmacro(or_add_test)


#
# creates the test case source files from the given input
#
macro(or_create_test_case declarative_test_target)
	#not implemented yet
endmacro(or_create_test_case)


#
# adds a test case from either a source file or a declarative test to a Test Target
# * test_case_target - the name of the Test Target
# a test target is not exactly the same as a CMake target, it is a collection of tests
# * TEST_SOURCES <test_sources> - source files which contain tests
# * DECLARATIVE_TESTS <declarative_test_targets> - declarative test targets that are created by macro or_add_test()
# * INCLUDES <targets> - targets from which include directories need to be included
# * DEPENDENCIES <dependencies> - dependencies of the test files
# the following variables will be set during the process
# * test_case_target_FILES - contains the source files that are transfered and those created from declarative tests
# * test_case_target_DEPENDENCIES - contains the dependencies of the test files
# * test_case_target_INCLUDES - contains the includes the test files need
#
macro(or_add_test_case test_case_target)
	message(STATUS "creating Test Target: ${test_case_target}")
	foreach(arg ${ARGV})
		if(_in_declarative_tests MATCHES "true" AND NOT(${arg} MATCHES "TEST_SOURCES") AND NOT(${arg} MATCHES "INCLUDES") AND NOT(${arg} MATCHES "DEPENDENCIES"))
			set(${test_case_target}_declarative_tests ${arg} ${${test_case_target}_declarative_tests})
		elseif(_in_test_case_files MATCHES "true" AND NOT(${arg} MATCHES "DECLARATIVE_TESTS") AND NOT(${arg} MATCHES "INCLUDES") AND NOT(${arg} MATCHES "DEPENDENCIES"))
			set(${test_case_target}_FILES "${arg}" ${${test_case_target}_FILES})
		elseif(_in_includes MATCHES "true" AND NOT(${arg} MATCHES "TEST_SOURCES") AND NOT(${arg} MATCHES "DECLARATIVE_TESTS") AND NOT(${arg} MATCHES "DEPENDENCIES"))
			set(_includes ${arg} ${_includes})
		elseif(_in_dependencies MATCHES "true" AND NOT(${arg} MATCHES "TEST_SOURCES") AND NOT(${arg} MATCHES "DECLARATIVE_TESTS") AND NOT(${arg} MATCHES "INCLUDES"))
			set(_dependencies ${arg} ${_dependencies})
		endif(_in_declarative_tests MATCHES "true" AND NOT(${arg} MATCHES "TEST_SOURCES") AND NOT(${arg} MATCHES "INCLUDES") AND NOT(${arg} MATCHES "DEPENDENCIES"))
		
		if(${arg} MATCHES "DECLARATIVE_TESTS")
			set(_in_declarative_tests "true")
			set(_in_test_case_files "false")
			set(_in_includes "false")
			set(_in_dependencies "false")
		elseif(${arg} MATCHES "TEST_SOURCES")
			set(_in_declarative_tests "false")
			set(_in_test_case_files "true")
			set(_in_includes "false")
			set(_in_dependencies "false")
		elseif(${arg} MATCHES "INCLUDES")
			set(_in_declarative_tests "false")
			set(_in_test_case_files "false")
			set(_in_includes "true")
			set(_in_dependencies "false")
		elseif(${arg} MATCHES "DEPENDENCIES")
			set(_in_declarative_tests "false")
			set(_in_test_case_files "false")
			set(_in_includes "false")
			set(_in_dependencies "true")
		endif(${arg} MATCHES "DECLARATIVE_TESTS")
	endforeach(arg)
	
	#create the Test Cases from Declarative Test Target if any are present
	list(LENGTH ${test_case_target}_declarative_tests _tmp)
	if(_tmp GREATER 0)
		or_create_test_case(${test_case_target}_declarative_tests)
	endif(_tmp GREATER 0)
	#put all together
	set(${test_case_target}_FILES ${${test_case_target}_declarative_tests_FILES} ${${test_case_target}_FILES})
	set(${test_case_target}_DEPENDENCIES ${_dependencies})
	set(${test_case_target}_INCLUDES ${_includes})
endmacro()


#
# summarize the test cases to generate a target for these Test Cases, add and link their dependencies and involve the includes
# the target that is created is a very CMake target
# if one or more of the dependencies is an executable target a new target will be created as a shared library using the same dependencies and source code files
# * test_target_id - the name of the target id that can be used to track the element, e.g. adding the element to a Test Suite
# * TEST_CASE_TARGETS <test case targets> - the test cases that should be part of this Test Target
# * SOURCE_DIR <source dir> - if a library needs to be created as a different type the source directory of that library needs to be mentioned here
#
macro(or_add_test_target test_target_id)
	message(STATUS "creating Target: ${test_target_id}")
	foreach(arg ${ARGV})
		if(_in_case_targets MATCHES "true" AND NOT(${arg} MATCHES "SOURCE_DIR") AND NOT(_in_source_dir MATCHES "true"))
			set(_test_case_targets ${arg} ${_test_case_targets})
		elseif(_in_source_dir MATCHES "true" AND NOT(${arg} MATCHES "TEST_CASE_TARGETS"))
			set(_test_case_targets_source_dir ${arg})
		endif(_in_case_targets MATCHES "true" AND NOT(${arg} MATCHES "SOURCE_DIR") AND NOT(_in_source_dir MATCHES "true"))
		
		if(${arg} MATCHES "TEST_CASE_TARGETS")
			set(_in_case_targets "true")
			set(_in_source_dir "false")
		elseif(${arg} MATCHES "SOURCE_DIR")
			#set(_in_case_targets "false")
			set(_in_source_dir "true")
		endif(${arg} MATCHES "TEST_CASE_TARGETS")
	endforeach(arg)
	
	list(LENGTH ${_test_case_targets}_FILES _tmp)
	if(_tmp GREATER 0)
		add_executable("${test_target_id}" ${${_test_case_targets}_FILES})						#add the test target as a new executable
		list(LENGTH ${_test_case_targets}_DEPENDENCIES _tmp)
		if(_tmp GREATER 0)
			foreach(arg ${${_test_case_targets}_DEPENDENCIES})
				get_target_property(var ${arg} OUTPUT_NAME)
				string(SUBSTRING ${var} 0 4 _start)
				if(${_start} MATCHES "App_")
					foreach(_elem ${${arg}_SOURCES})
						string(REGEX REPLACE "${_elem}" "${_test_case_targets_source_dir}/${_elem}" _tmpElem ${_elem})
						list(APPEND ${arg}_LIB_SOURCES ${_tmpElem})
					endforeach(_elem)
					message(STATUS "creating library ${arg}_LIB from executable target ${arg}")
					or_add_library(${arg}_LIB SHARED ${${arg}_LIB_SOURCES})						#add the new library used for testing
					set(${arg}_LIB_INCLUDE_DIR ${_test_case_targets_source_dir})				#since the header files of applications are located in the same directory as the source this variable has to be corrected to not locate /include
					or_add_dependencies(${arg}_LIB ${${_test_case_targets}_DEPENDENCIES})		#make the new library depend
					or_add_includes(${arg}_LIB ${${test_case_targets}_INCLUDES})
					list(REMOVE_ITEM ${_test_case_targets}_DEPENDENCIES ${arg})					#remove the old library...
					list(APPEND ${_test_case_targets}_DEPENDENCIES ${arg}_LIB)					#and add the new one
				endif(${_start} MATCHES "App_")
			endforeach(arg)
         or_add_dependencies(${test_target_id} ${${_test_case_targets}_DEPENDENCIES})
         OR_ADD_EXTERNAL_LIBRARY(${test_target_id} ${Boost_LIBRARIES})
		else(_tmp GREATER 0)
			OR_ADD_EXTERNAL_LIBRARY(${test_target_id} ${Boost_LIBRARIES})
		endif(_tmp GREATER 0)
		list(LENGTH ${_test_case_targets}_INCLUDES _tmp)
		if(_tmp GREATER 0)
			or_add_includes( ${test_target_id} ${${_test_case_targets}_INCLUDES})
		endif(_tmp GREATER 0)
	else(_tmp GREATER 0)
		message(SEND_ERROR "No Test Cases have been found")
	endif(_tmp GREATER 0)
endmacro(or_add_test_target)

#
# prepares the Test Target of the test for the Test Runner
# * test_suite_target - the name of the target that will be used to create the target for the Test Runner
# * TEST_TARGETS <test targets> - the Test Targets that should be part of this Test Suite
# * PARAMETERS <parameters> - the parameters that should be used when executing the Boost Test Runner
#
macro(or_add_test_suite test_suite_target)
	foreach(arg ${ARGV})
		if(_in_parameters MATCHES "true" AND NOT(${arg} MATCHES "TEST_TARGETS"))
			string(SUBSTRING ${arg} 0 2 _beg)
			if(${_beg} MATCHES "--")
				set(_parameters "${arg} ${_parameters}")
			else(${_beg} MATCHES "--")
				set(_parameters "--${arg} ${_parameters}")
			endif(${_beg} MATCHES "--")
		elseif(_in_targets MATCHES "true" AND NOT(${arg} MATCHES "PARAMETERS"))
			set(_test_targets ${arg} ${_test_targets})
		endif(_in_parameters MATCHES "true" AND NOT(${arg} MATCHES "TEST_TARGETS"))
		
		if(${arg} MATCHES "PARAMETERS")
			set(_in_parameters "true")
		elseif((${arg} MATCHES "TEST_TARGETS"))
			set(_in_targets "true")
		endif(${arg} MATCHES "PARAMETERS")
	endforeach(arg)
	
	list(LENGTH _test_targets _tmp)
	if(_tmp GREATER 0)								#if there are Test Targets
		message(STATUS "adding ${_test_targets} to Test Suite ${test_suite_target}")
		if(${CMAKE_GENERATOR} MATCHES "Visual Studio.*")
			foreach(target ${_test_targets})			#create a command for each of them so the Test Targets are executed
				add_custom_command(	TARGET "${_test_targets}"
									POST_BUILD
									COMMAND "$(TargetDir)$(TargetName)${CMAKE_EXECUTABLE_SUFFIX}"
									ARGS	"${_parameters} --report XML"
									)
			endforeach(target)
		else(${CMAKE_GENERATOR} MATCHES "Visual Studio.*")
			foreach(target ${_test_targets})			#create a command for each of them so the Test Targets are executed
				add_custom_command(	TARGET "${_test_targets}"
									POST_BUILD
									COMMAND "${EXECUTABLE_OUTPUT_PATH}/${target}${CMAKE_EXECUTABLE_SUFFIX}"
									ARGS	"${_parameters} --report XML"
									)
			endforeach(target)
		endif(${CMAKE_GENERATOR} MATCHES "Visual Studio.*")
		or_project_group(${_test_targets} "Tests/${test_suite_target}")
	else(_tmp GREATER 0)
		message(SEND_ERROR "No Test Targets have been found")
	endif(_tmp GREATER 0)
endmacro(or_add_test_suite)