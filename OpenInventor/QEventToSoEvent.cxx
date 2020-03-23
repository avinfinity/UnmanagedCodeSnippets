#include <QEventToSoEvent.h>
#include <Inventor/SoDB.h>

// static members
SoEventBuilder QEventToSoEvent::m_ivEvent; 
bool QEventToSoEvent::s_init = QEventToSoEvent::initClass();

int QEventToSoEvent::keyMapInitFlag = 0;
SoKeyboardEvent::Key QEventToSoEvent::keyMap[256];
SoKeyboardEvent::Key QEventToSoEvent::keyMap2[97];

#define Q_KEY_OFFSET 0x1000000


bool
QEventToSoEvent::initClass() 
{
  // initialize virtual key mapping just once
  if ( !keyMapInitFlag ) 
  {
    keyMapInitFlag = 1;

    int i;
    for (i = 0; i < 256; i++)
      keyMap[i] = SoKeyboardEvent::ANY;

    keyMap[Qt::Key_Space] = SoKeyboardEvent::SPACE;

    // Number keys
    keyMap[Qt::Key_0] = SoKeyboardEvent::NUMBER_0;
    keyMap[Qt::Key_1] = SoKeyboardEvent::NUMBER_1;
    keyMap[Qt::Key_2] = SoKeyboardEvent::NUMBER_2;
    keyMap[Qt::Key_3] = SoKeyboardEvent::NUMBER_3;
    keyMap[Qt::Key_4] = SoKeyboardEvent::NUMBER_4;
    keyMap[Qt::Key_5] = SoKeyboardEvent::NUMBER_5;
    keyMap[Qt::Key_6] = SoKeyboardEvent::NUMBER_6;
    keyMap[Qt::Key_7] = SoKeyboardEvent::NUMBER_7;
    keyMap[Qt::Key_8] = SoKeyboardEvent::NUMBER_8;
    keyMap[Qt::Key_9] = SoKeyboardEvent::NUMBER_9;

    // Alphabetic keys
    keyMap[Qt::Key_A] = SoKeyboardEvent::A;
    keyMap[Qt::Key_B] = SoKeyboardEvent::B;
    keyMap[Qt::Key_C] = SoKeyboardEvent::C;
    keyMap[Qt::Key_D] = SoKeyboardEvent::D;
    keyMap[Qt::Key_E] = SoKeyboardEvent::E;
    keyMap[Qt::Key_F] = SoKeyboardEvent::F;
    keyMap[Qt::Key_G] = SoKeyboardEvent::G;
    keyMap[Qt::Key_H] = SoKeyboardEvent::H;
    keyMap[Qt::Key_I] = SoKeyboardEvent::I;
    keyMap[Qt::Key_J] = SoKeyboardEvent::J;
    keyMap[Qt::Key_K] = SoKeyboardEvent::K;
    keyMap[Qt::Key_L] = SoKeyboardEvent::L;
    keyMap[Qt::Key_M] = SoKeyboardEvent::M;
    keyMap[Qt::Key_N] = SoKeyboardEvent::N;
    keyMap[Qt::Key_O] = SoKeyboardEvent::O;
    keyMap[Qt::Key_P] = SoKeyboardEvent::P;
    keyMap[Qt::Key_Q] = SoKeyboardEvent::Q;
    keyMap[Qt::Key_R] = SoKeyboardEvent::R;
    keyMap[Qt::Key_S] = SoKeyboardEvent::S;
    keyMap[Qt::Key_T] = SoKeyboardEvent::T;
    keyMap[Qt::Key_U] = SoKeyboardEvent::U;
    keyMap[Qt::Key_V] = SoKeyboardEvent::V;
    keyMap[Qt::Key_W] = SoKeyboardEvent::W;
    keyMap[Qt::Key_X] = SoKeyboardEvent::X;
    keyMap[Qt::Key_Y] = SoKeyboardEvent::Y;
    keyMap[Qt::Key_Z] = SoKeyboardEvent::Z;

    // Numeric pad
    keyMap[Qt::Key_0] = SoKeyboardEvent::PAD_0;
    keyMap[Qt::Key_1] = SoKeyboardEvent::PAD_1;
    keyMap[Qt::Key_2] = SoKeyboardEvent::PAD_2;
    keyMap[Qt::Key_3] = SoKeyboardEvent::PAD_3;
    keyMap[Qt::Key_4] = SoKeyboardEvent::PAD_4;
    keyMap[Qt::Key_5] = SoKeyboardEvent::PAD_5;
    keyMap[Qt::Key_6] = SoKeyboardEvent::PAD_6;
    keyMap[Qt::Key_7] = SoKeyboardEvent::PAD_7;
    keyMap[Qt::Key_8] = SoKeyboardEvent::PAD_8;
    keyMap[Qt::Key_9] = SoKeyboardEvent::PAD_9;

    keyMap[Qt::Key_Asterisk] = SoKeyboardEvent::PAD_MULTIPLY;
    keyMap[Qt::Key_Plus ] = SoKeyboardEvent::PAD_ADD;
    keyMap[Qt::Key_Minus ] = SoKeyboardEvent::PAD_SUBTRACT;
    keyMap[Qt::Key_Slash ] = SoKeyboardEvent::PAD_DIVIDE;

    // The following do not have Windows VK_xxx values
    keyMap[Qt::Key_Semicolon]   = SoKeyboardEvent::SEMICOLON;
    keyMap[Qt::Key_Equal]       = SoKeyboardEvent::EQUAL;
    keyMap[Qt::Key_Comma]       = SoKeyboardEvent::COMMA;
    keyMap[Qt::Key_Minus]       = SoKeyboardEvent::MINUS;
    keyMap[Qt::Key_Period]      = SoKeyboardEvent::PERIOD;
    keyMap[Qt::Key_Slash]       = SoKeyboardEvent::SLASH;
    keyMap[Qt::Key_AsciiTilde]  = SoKeyboardEvent::GRAVE;
    keyMap[Qt::Key_BracketLeft] = SoKeyboardEvent::BRACKETLEFT;
    keyMap[Qt::Key_Backslash]   = SoKeyboardEvent::BACKSLASH;
    keyMap[Qt::Key_BracketRight]= SoKeyboardEvent::BRACKETRIGHT;
    keyMap[Qt::Key_Apostrophe]  = SoKeyboardEvent::APOSTROPHE;

    //keymap2 for some reason some qt key are greater than 0x1000000
    for (i = 0; i < 97; i++)
      keyMap2[i] = SoKeyboardEvent::ANY;

    // Function keys
    keyMap2[Qt::Key_F1 - Q_KEY_OFFSET] = SoKeyboardEvent::F1 ;
    keyMap2[Qt::Key_F2 - Q_KEY_OFFSET] = SoKeyboardEvent::F2 ;
    keyMap2[Qt::Key_F3 - Q_KEY_OFFSET] = SoKeyboardEvent::F3 ;
    keyMap2[Qt::Key_F4 - Q_KEY_OFFSET] = SoKeyboardEvent::F4 ;
    keyMap2[Qt::Key_F5 - Q_KEY_OFFSET] = SoKeyboardEvent::F5 ;
    keyMap2[Qt::Key_F6 - Q_KEY_OFFSET] = SoKeyboardEvent::F6 ;
    keyMap2[Qt::Key_F7 - Q_KEY_OFFSET] = SoKeyboardEvent::F7 ;
    keyMap2[Qt::Key_F8 - Q_KEY_OFFSET] = SoKeyboardEvent::F8 ;
    keyMap2[Qt::Key_F9 - Q_KEY_OFFSET] = SoKeyboardEvent::F9 ;
    keyMap2[Qt::Key_F10 - Q_KEY_OFFSET] = SoKeyboardEvent::F10;
    keyMap2[Qt::Key_F11 - Q_KEY_OFFSET] = SoKeyboardEvent::F11;
    keyMap2[Qt::Key_F12 - Q_KEY_OFFSET] = SoKeyboardEvent::F12;

    keyMap2[Qt::Key_Backspace  - Q_KEY_OFFSET] = SoKeyboardEvent::BACKSPACE;
    keyMap2[Qt::Key_Tab   - Q_KEY_OFFSET] = SoKeyboardEvent::TAB;
    keyMap2[Qt::Key_Return  - Q_KEY_OFFSET] = SoKeyboardEvent::RETURN;
    keyMap2[Qt::Key_Enter  - Q_KEY_OFFSET] = SoKeyboardEvent::PAD_ENTER;
    keyMap2[Qt::Key_Shift  - Q_KEY_OFFSET] = SoKeyboardEvent::LEFT_SHIFT;
    keyMap2[Qt::Key_Control - Q_KEY_OFFSET] = SoKeyboardEvent::LEFT_CONTROL;
    keyMap2[Qt::Key_Alt - Q_KEY_OFFSET] = SoKeyboardEvent::LEFT_ALT;
    keyMap2[Qt::Key_Menu - Q_KEY_OFFSET] = SoKeyboardEvent::LEFT_ALT;
    keyMap2[Qt::Key_Pause - Q_KEY_OFFSET] = SoKeyboardEvent::PAUSE;
    keyMap2[Qt::Key_CapsLock - Q_KEY_OFFSET] = SoKeyboardEvent::CAPS_LOCK;
    keyMap2[Qt::Key_Escape - Q_KEY_OFFSET] = SoKeyboardEvent::ESCAPE;
    keyMap2[Qt::Key_PageUp  - Q_KEY_OFFSET] = SoKeyboardEvent::PAGE_UP;
    keyMap2[Qt::Key_PageDown - Q_KEY_OFFSET] = SoKeyboardEvent::PAGE_DOWN;
    keyMap2[Qt::Key_End - Q_KEY_OFFSET] = SoKeyboardEvent::END;
    keyMap2[Qt::Key_Home - Q_KEY_OFFSET] = SoKeyboardEvent::HOME;
    keyMap2[Qt::Key_Left - Q_KEY_OFFSET] = SoKeyboardEvent::LEFT_ARROW;
    keyMap2[Qt::Key_Up - Q_KEY_OFFSET] = SoKeyboardEvent::UP_ARROW;
    keyMap2[Qt::Key_Right - Q_KEY_OFFSET] = SoKeyboardEvent::RIGHT_ARROW;
    keyMap2[Qt::Key_Down - Q_KEY_OFFSET] = SoKeyboardEvent::DOWN_ARROW;
    keyMap2[Qt::Key_Print - Q_KEY_OFFSET] = SoKeyboardEvent::PRINT;
    keyMap2[Qt::Key_Insert - Q_KEY_OFFSET] = SoKeyboardEvent::INSERT;
    keyMap2[Qt::Key_Delete - Q_KEY_OFFSET] = SoKeyboardEvent::KEY_DELETE;
    keyMap2[Qt::Key_NumLock - Q_KEY_OFFSET] = SoKeyboardEvent::NUM_LOCK;
    keyMap2[Qt::Key_ScrollLock - Q_KEY_OFFSET] = SoKeyboardEvent::SCROLL_LOCK;
    keyMap2[Qt::Key_Meta - Q_KEY_OFFSET] = SoKeyboardEvent::LEFT_META; // Key_Meta has the same value for LWIN and RWIN
  }
  return true;
}

SoKeyboardEvent::Key 
QEventToSoEvent::getIvKey(QKeyEvent *qevent)
{
  SoKeyboardEvent::Key ivKey;
  int rawKey = (int)qevent->key();
  if (rawKey >= Q_KEY_OFFSET) 
  {
    rawKey -= Q_KEY_OFFSET;
    ivKey = keyMap2[rawKey];
  }
  else
    ivKey = keyMap[rawKey];
  return ivKey;
}

SoMouseButtonEvent::Button 
QEventToSoEvent::getButtonId(QMouseEvent* qevent)
{
  SoMouseButtonEvent::Button whichButton;
  switch (qevent->button()) 
  {
  case Qt::LeftButton: whichButton = SoMouseButtonEvent::BUTTON1; break;
  case Qt::MidButton:  whichButton = SoMouseButtonEvent::BUTTON2; break;
  case Qt::RightButton:  whichButton = SoMouseButtonEvent::BUTTON3; break;
  default: whichButton = SoMouseButtonEvent::ANY;
  }
  return whichButton;
}

const std::vector<const SoEvent*>& 
QEventToSoEvent::getTouchEvents(QTouchEvent *qevent, QWidget* wsource )
{
  QList<QTouchEvent::TouchPoint> touchPoints = qevent->touchPoints();
  m_soeventlist.clear();
  for(int i=0; i<touchPoints.size(); i++)
  {
    int x = touchPoints[i].pos().x();
    int y = (wsource->height()-1) - touchPoints[i].pos().y();
    if (touchPoints[i].state() == Qt::TouchPointPressed)
    {
      const std::vector<const SoEvent*>& partialeventlist = m_ivEvent.getTouchDownEvent(x,y, touchPoints[i].id());
      m_soeventlist.insert(m_soeventlist.end(), partialeventlist.begin(), partialeventlist.end());
    }
    else if(touchPoints[i].state() == Qt::TouchPointMoved)
    {
      const std::vector<const SoEvent*>& partialeventlist = m_ivEvent.getTouchMoveEvent(x,y, touchPoints[i].id());
      m_soeventlist.insert(m_soeventlist.end(), partialeventlist.begin(), partialeventlist.end());
    }
    else if (touchPoints[i].state() == Qt::TouchPointReleased)
    {
      const std::vector<const SoEvent*>& partialeventlist = m_ivEvent.getTouchUpEvent(x,y, touchPoints[i].id());
      m_soeventlist.insert(m_soeventlist.end(), partialeventlist.begin(), partialeventlist.end());
    }
  }
  return m_soeventlist;
}
