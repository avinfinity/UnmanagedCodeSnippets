# Destination Platform chooser

if(CYGWIN)
	message(STATUS "Compiling for CYGWIN platform...")
	if(EXISTS Platform.cygwin.cmake)
		include( Platform.cygwin.cmake )
	endif(EXISTS Platform.cygwin.cmake)
else(CYGWIN)
	if(WIN32)
		message(STATUS "Compiling for WIN32 platform...")
		if(EXISTS Platform.win32.cmake)
			include( Platform.win32.cmake )
		endif(EXISTS Platform.win32.cmake)
	else(WIN32)
		if(APPLE)
			message(STATUS "Compiling for APPLE platform...")
			if(EXISTS Platform.apple.cmake)
				include( Platform.apple.cmake )
			endif(EXISTS Platform.apple.cmake)
		else(APPLE)
			if(UNIX)
				message(STATUS "Compiling for UNIX platform...")
				if(EXISTS Platform.unix.cmake)
					include( Platform.unix.cmake )
				endif(EXISTS Platform.unix.cmake)
			endif(UNIX)
		endif(APPLE)
	endif(WIN32)
endif(CYGWIN)