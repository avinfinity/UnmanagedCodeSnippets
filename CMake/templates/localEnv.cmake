# This is a template for the localeEnv.cmake file. Customize this to your appropiate paths
# and copy it where your top level CMakeLists.txt is.

# Should the VS8 project- and solution-files be rebuilt when cmake is called (with -G "NMake VS8")?
SET(REBUILD_VS_FILES FALSE)

# Some required global paths
SET(SOURCEROOT ..)
set(BUILDAREADIR "C:/MW/ninja/BuildArea/OpenOR")
SET(TOOLROOTDIR C:/MW/ThirdParty)

SET(REBUILD_VS_FILES FALSE)

SET(SOURCEROOT ..)
set(BUILDAREADIR "C:/MW/ninja/BuildArea/openOR")
set(TOOLROOTDIR C:/MW/ThirdParty)

SET(QTDIR         ${TOOLROOTDIR}/QT/4.2.2)
GET_FILENAME_COMPONENT(QTDIR ${QTDIR} ABSOLUTE)

SET(BOOSTDIR      ${TOOLROOTDIR}/BOOST/1_34_0)
GET_FILENAME_COMPONENT(BOOSTDIR ${BOOSTDIR} ABSOLUTE)

SET(OPENMESHDIR   ${TOOLROOTDIR}/OPENMESH/1.1.0)
GET_FILENAME_COMPONENT(OPENMESHDIR ${OPENMESHDIR} ABSOLUTE)

SET(DCMTKDIR      ${TOOLROOTDIR}/DCMTK/3.5.4)
GET_FILENAME_COMPONENT(DCMTKDIR ${DCMTKDIR} ABSOLUTE)

SET(CPPUNITDIR    ${TOOLROOTDIR}/CPPUNIT/1.12.0)
GET_FILENAME_COMPONENT(CPPUNITDIR ${CPPUNITDIR} ABSOLUTE)

SET(UPXDIR        ${TOOLROOTDIR}/UPX)
GET_FILENAME_COMPONENT(UPXDIR ${UPXDIR} ABSOLUTE)

# The generator to create the build files for openOR. This is a platform-dependent setting!
set(orGENERATOR "NMake Makefiles VS8")

# For "developer-mode" skip missing files. Optional_Installs should be set to "" for release-builds!
set(Optional_Installs "OPTIONAL")
