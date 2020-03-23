//****************************************************************************
// (c) 2008 by the openOR Team
//****************************************************************************
// The contents of this file are available under the GPL v2.0 license
// or under the openOR comercial license. see
//   /Doc/openOR_free_license.txt or
//   /Doc/openOR_comercial_license.txt
// for Details.
//****************************************************************************
//! OPENOR_INTERFACE_FILE(Basic)
//! \file
//! \ingroup Basic
//****************************************************************************
/** 
 * file
 * author Christian Winne
 */
#ifndef openOR_ProgramState_hpp
#define openOR_ProgramState_hpp

#include <openOR/Plugin/CreateInterface.hpp>

namespace openOR {

   
    //! \brief  ProgramState interface.
    //! \note   To use ProgramState objects include <tt><openOR/ProgramStateImpl.hpp></tt>.
    //!         Further a dependency to \c openOR_Basic must be added to the
    //!         appropriate \c CMakeLists.txt file.
    //!
    //! Breaking programms down into different states this class defines an
    //! interface representing a program's state. Every time the state changes
    //! (or is changed) an update signal is emitted.
    //! 
    //! \b Example:
    //! 
    //! \code
    //! #include <openOR/ProgramStateImpl.hpp>
    //! #include <openOR/Utility/main.hpp>
    //!
    //! class Program {
    //!
    //!     public:
    //!         typedef enum {
    //!             INITIAL, PLANNING, STATE_X
    //!         } State;
    //!         
    //!         // constructor
    //!         Program() {
    //!             // construct state
    //!             m_pState = createInstanceOf<ProgramStateImpl>();
    //!
    //!             // connect to state signal
    //!             openOR::connect<void()>(interface_cast<Signaller>(m_pState), "updated", boost::bind(&Program::setState, this)); 
    //!
    //!             // set initial state (calls setState())
    //!             m_pState->setState(INITIAL);
    //!         }
    //!
    //!     private:
    //!         void setState() {
    //!             // get new state
    //!             unsigned int s = m_pState->currentState();
    //!                 
    //!             // react to state change and e.g. change view
    //!         }
    //!
    //!         std::shared_ptr<ProgramStateImpl> m_pState;
    //! };
    //! \endcode
    struct ProgramState {

        virtual void setState(unsigned int nState) = 0;
        virtual unsigned int currentState() const = 0;

    };

}

/************************************************************************/
/*                                                                      */
/************************************************************************/


OPENOR_CREATE_INTERFACE(openOR::ProgramState)
void setState(unsigned int nState) { adaptee()->setState(nState); }
unsigned int currentState() const { return adaptee()->currentState(); }
OPENOR_CREATE_INTERFACE_END


#endif 
