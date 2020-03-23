#-------------------------------------------------------------------------------------------------------------------------------
# (c) 2012 by the openOR Team
#-------------------------------------------------------------------------------------------------------------------------------
# The contents of this file are available under the GPL v2.0 license
# or under the openOR comercial license. see
#   /Doc/openOR_free_license.txt or
#   /Doc/openOR_comercial_license.txt
# for Details.
#-------------------------------------------------------------------------------------------------------------------------------

openOR_source(Basic
   INTERFACE ID 02_ExpHeader NAME "Interface Headers"
   include/openOR/Defs/Basic.hpp
   #include/openOR/AttentiveContainer.hpp
   #include/openOR/AttentiveContainerImpl.hpp
   #include/openOR/SignallerImpl.hpp   
   include/openOR/Utility/ProgressBase.hpp
)

openOR_source(Basic
   INTERFACE ID 01_Interfaces NAME "Interfaces"
   include/openOR/Callable.hpp
   #include/openOR/Lockable.hpp
   include/openOR/Progressable.hpp
   include/openOR/Cancelable.hpp
   #include/openOR/Updatable.hpp
   #include/openOR/Initializer.hpp
   include/openOR/DataSettable.hpp
   include/openOR/Signaller.hpp
   include/openOR/ProgramState.hpp
   #include/openOR/Validatable.hpp
   #include/openOR/StartStopable.hpp
   include/openOR/PropertyAccess.hpp
   #include/openOR/Manipulatable.hpp
   #include/openOR/ProgramStateImpl.hpp
   include/openOR/UpdateSignalCallable.hpp
)

openOR_source(Basic
   IMPL ID 03_Impl NAME "Source"
   #AttentiveContainer.cpp
   ProgramStateImpl.cpp
   UpdateSignalCallable.cpp
   ProgressBase.cpp
)

openOR_source(Basic
   IMPL ID 04_Factory NAME "Plugin Config"

   Basic_plugin_factory.cpp
   ${Basic_HEADERS};
   ${Basic_SOURCES};
   ${Basic_INTERFACES};
)

openOR_add_dependency(Basic openOR_core)
#openOR_add_dependency(Basic Boost)
#openOR_add_dependency(Basic Utility_BoostExtensions)

openOR_create(LIBRARY Basic GROUP "Core")
