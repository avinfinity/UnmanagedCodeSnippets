#
# This little helper Macro gets the subversion 
# Revision number if possible, thus you can tag 
# your build with it. Returns the variable ${BuildID}
#

macro(GET_SVN_REVISION)
	set( BuildID "" )
	find_program(_SVN_VERSION_EXEC
		NAMES svnversion svnversion.exe
		PATHS 
			/usr/local/bin
	)
	#message(" svnvers exec: ${_SVN_VERSION_EXEC}")
	if (_SVN_VERSION_EXEC)
		execute_process( COMMAND ${_SVN_VERSION_EXEC} -n ${CMAKE_SOURCE_DIR}
			OUTPUT_VARIABLE _SVN_REV
		)
		set( BuildID "r${_SVN_REV}" )
		#message ("Last SVN checkout: ${BuildID}")
	endif (_SVN_VERSION_EXEC)
endmacro(GET_SVN_REVISION)