================================================================================
                      PROJECT: MHOOK - Structure Description
================================================================================

General Information:
--------------------
MHook - Windows application for emulating keyboard presses via
mouse and eye-trackers (Tobii REX, TheEyeTribe)
Platform: Windows (Win32 API)
Architecture: "Strategy" Pattern with central abstract class

================================================================================
                            DIRECTORY STRUCTURE
================================================================================

C:\Projects\mhook\source mhook\
  .vs\                           Visual Studio settings
  x64\Debug\                     Compiled objects (ignore)
  MHook64.sln                    Solution file
  MHook64.vcxproj                Project file
  MHook64.vcxproj.filters       Project filters
  resource.rc                    Application resources

================================================================================
                              PROJECT FILES
================================================================================

HEADER FILES (.h):
------------------
HookHandler.h          Base abstract class for hook handlers
hh1.h, hh1a.h          Modes 1 and 1a (standard)
hh2.h                  Mode 2 (direction preview)
hh3.h                  Mode 3 (fast movement filtering)
hh4.h                  Mode 4 (gas/turn independently)
hh5.h                  Mode 5 (8 keys, auto-click)
hh6.h                  Mode 6 (scroll mode)
hh7.h                  Mode 7 (empty mode)
MHKeypad.h             Keyboard module
MVector.h              Vector module for movement analysis
Settings.h             Settings module
MagicWindow.h          Magic windows
Bitmap.h               Graphics resources
TobiiREX.h             Eye-tracker Tobii REX
TET.h                  Eye-tracker TheEyeTribe
CircleWindow.h         Circle window (indicator)
MHRepErr.h             Error handling
WM_USER_messages.h     Windows user messages
CursorDot.h            Dot cursor (for eye-tracker)
resource.h             Resource definitions

SOURCE FILES (.cpp):
--------------------
HookHandler.cpp        Base MHookHandler implementation
hh1.cpp - hh6.cpp      Mode implementations 1-6
MHKeypad.cpp           Keyboard module implementation
MVector.cpp            Vector module implementation
Settings.cpp           Main settings dialog
Settings2.cpp          Magic windows dialog
MagicWindow.cpp        Magic windows implementation
Bitmap.cpp             Graphics implementation
TobiiREX.cpp           Tobii REX implementation
TET.cpp                TheEyeTribe implementation
CircleWindow.cpp       Circle window implementation
HookProc.cpp           Main mouse hook procedure
OnGazeData.cpp         Eye-tracker data processing
MHRepErr.cpp           Error handling implementation
MH001.cpp, MH002.cpp   Application entry points

RESOURCES:
----------
resource.rc            Resource file
resource.h             Resource header
bm4w.bmp               4 directions (white)
bm4wred.bmp            4 directions (red)
bm8w.bmp               8 directions (white)
bm8wred.bmp            8 directions (red)

================================================================================
                            CLASSES AND HIERARCHY
================================================================================

1. HOOK HANDLER HIERARCHY (Strategy Pattern)
--------------------------------------------

BASE CLASS: MHookHandler
  +-- MHookHandler1      (Mode 1 - standard)
  +-- MHookHandler1a     (Mode 1a - with timeout)
  +-- MHookHandler2      (Mode 2 - preview)
  +-- MHookHandler3      (Mode 3 - speed filtering)
  +-- MHookHandler4      (Mode 4 - gas/turn)
  +-- MHookHandler5      (Mode 5 - 8 keys, auto-click)
  +-- MHookHandler6      (Mode 6 - scroll)
  +-- MHookHandler7      (Mode 7 - empty)

MHookHandler Fields:
  rbutton_pressed      bool    right button state
  initialized          bool    initialization flag
  dx, dy               LONG    accumulated coordinates
  last_x, last_y       LONG    last coordinates
  position_mem         int     memorized position
  last_button5_time    DWORD   last 5th button time
  mouse_path_squared   int     accumulated mouse path

MHookHandler Virtual Methods:
  OnMouseMove(LONG x, LONG y)     int     process mouse movement
  OnMouseScroll(LONG x, LONG y)   void    process scroll
  OnRDown()                       bool    right button down
  OnRUp()                         bool    right button up
  OnLDown()                       void    left button down
  OnLUp()                         void    left button up
  GetPosition()                   int     get current position
  OnTimer()                       void    timer handler
  OnDraw(HDC, LONG)               void    render UI
  Halt()                          void    stop
  HaltGeneral()                   void    general reset
  Deinitialize()                  void    deinitialize
  TopLeftCornerTimer()            void    mode switcher
  OnFastMove(LONG dx, LONG dy)    void    handle fast movement

2. KEYBOARD MODULE: MHKeypad (static class)
-------------------------------------------
Methods:
  Init(int* scancodes, int extra_scancodes[5])    int   initialize
  Reset(int shift=0)                              void  reset all keys
  GetPosition()                                   int   get position
  Press(int pos, bool down, int shift=0)          void  press key (4/8)
  Press4(int pos, bool down, int shift=0)         void  press 1 of 4
  Press8(int pos, bool down)                      void  press 1 of 8

Fields:
  keypad_position   int     current position (-1 = no press)
  scancode[17]      int     array of key scan codes

3. VECTOR MODULE: MHVector (static class)
-----------------------------------------
Methods:
  Reset()                         void  reset vector
  NewValues(LONG dx, LONG dy)     int   process movement

Returns:
  0-7 : direction position
  -1  : direction not changed
  -2  : not enough movement

4. SETTINGS MODULE: MHSettings (static class)
---------------------------------------------
Fields:
  hh                          MHookHandler*   current handler
  mode                        int             current mode (1-7)
  num_positions               int             number of positions (4/8)
  mouse_sensitivity           int             mouse sensitivity
  timeout_after_move          DWORD           timeout after movement
  minimal_mouse_speed         LONG            minimum speed
  deadx, deady                LONG            dead zone
  flag_enable_speed_button    bool            speed button
  flag_2moves                 bool            double movement
  flag_alt2                   bool            alternative layout
  flag_right_mb_iskey         bool            right button as key
  flag_mode5autoclick         bool            mode 5 auto-click
  magic_wnd[24]               MagicWindow     magic windows array

Methods:
  SettingsDialogue(HWND)                    int    main dialog
  OpenMHookConfig()                         void   load configuration
  SaveMHookConfig()                         void   save configuration
  GetNumPositions()/SetNumPositions()       int/void   number of positions
  GetMouseSensitivity()/SetMouseSensitivity()  int/void   sensitivity

5. MAGIC WINDOWS: MagicWindow (class)
-------------------------------------
Instance Fields:
  myindex              int     window index (0-23)
  MWhwnd               HWND    window handle
  active               bool    is active
  mw_name[256]         TCHAR   name
  mw_color             int     color (0=green,1=yellow,2=red,3=blue)
  x, y, width, height  int     coordinates and size
  button_or_switch     int     type (0=button, 1=switch)
  mouse_or_eytracker   int     source (0=mouse, 1=eye-tracker)
  button_index         int     key index (0-103)
  mw_group             int     group (0-4)
  pressed              bool    is pressed
  f_inside_window      bool    cursor inside

Static Methods:
  Init()                          void   create 24 windows
  ShowEditable()                  void   edit mode
  ShowRuntime()                   void   runtime mode
  Hide()                          void   hide windows
  OnTimer5()                      void   mouse movement timer
  Press()                         void   press/release key
  PressSpecial(BYTE operation)    void   special operations

6. EYE-TRACKERS
---------------

a) BKBTobiiREX (Tobii REX, Tobii 4C, EyeX)
   Init(HWND)                      int    initialize
   Halt(HWND)                      void   stop
   Loads tobii_stream_engine.dll

b) BKBTET (TheEyeTribe)
   Init(HWND)                      int    TCP connection (port 6555)
   Halt(HWND)                      void   stop
   heartbeat_thread                thread connection keep-alive

Data Structures:
  toit_gaze_data:
    timestamp       uint64_t
    toit_status     int
    left, right     toit_eye (eye_data)

7. GRAPHICS MODULES
-------------------

a) MHBitmap (static class)
   Init(HWND)                      void   load bitmaps
   Halt()                          void   free resources
   OnDraw(HDC, int position)       void   render position

b) CircleWindow (static class)
   Init()                          void   create window
   Show()                          void   show
   Hide()                          void   hide
   CircleHwnd                      HWND   window handle

8. HELPER MODULES
-----------------

a) MHRepErr (error handling)
   MHReportError(TCHAR* file, TCHAR* func, int line, HWND)
   MHReportError(TCHAR* error, HWND)
   MHReportError(int tobii_error_code, ...)

b) HookProc (global function)
   LRESULT CALLBACK HookProc(int disabled, WPARAM, LPARAM)
   Handles: WM_MOUSEMOVE, WM_RBUTTONDOWN/UP, WM_LBUTTONDOWN/UP

c) on_gaze_data (global function)
   void on_gaze_data(toit_gaze_data* data, void* user_data)
   Eye-tracker data processing

================================================================================
                              OPERATING MODES
================================================================================

Mode 1 (MHookHandler1):
  Standard emulation mode
  Double movement (flag_2moves_mode1)
  Two alternative layouts (flag_alt2)
  Direction change on the fly

Mode 1a (MHookHandler1a):
  Simplified version of mode 1
  Timeout between presses

Mode 2 (MHookHandler2):
  Direction preview
  Mouse movement shows direction
  Right button press activates key

Mode 3 (MHookHandler3):
  Fast movement filtering
  Mouse speed analysis
  serial_fasts, oast_allowed, b2st_allowed

Mode 4 (MHookHandler4):
  Gas/turn independently
  Independent X and Y axis control
  One axis can work in mode 3

Mode 5 (MHookHandler5):
  8 independent keys (Press8)
  Works only with right button pressed
  Auto-click on movement
  Wheel selection (circle_scale_factor)

Mode 6 (MHookHandler6):
  Scroll mode
  Mouse wheel emulation
  Overrides OnMouseScroll

Mode 7 (MHookHandler7):
  Empty mode
  Windows only without mouse processing
  Pass-through mode

================================================================================
                           MODULE DEPENDENCIES
================================================================================

MHookHandler (abstract)
    +-- MHookHandler1  --> MHVector, MHKeypad, MHSettings
    +-- MHookHandler1a --> MHVector, MHKeypad
    +-- MHookHandler2  --> MHVector, MHKeypad
    +-- MHookHandler3  --> MHVector, MHKeypad
    +-- MHookHandler4  --> MHKeypad, MVector
    +-- MHookHandler5  --> MHVector, MHKeypad
    +-- MHookHandler6  --> MHVector, MHKeypad (overrides OnMouseScroll)
    +-- MHookHandler7  (empty implementation)

MHSettings
    +-- Contains: MHookHandler* hh (pointer to current handler)
    +-- Contains: instances hh1-hh7
    +-- Uses: MHKeypad, MagicWindow, MHVector
    +-- Dialog functions work with resources

HookProc
    +-- Calls: MHSettings::hh->OnMouseMove/OnRDown/OnRUp/OnMouseScroll
    +-- Calls: MHSettings::hh->OnLDown/OnLUp
    +-- Manages: flag_stop_emulation, flag_left_button_key

MagicWindow
    +-- Uses: MHRepErr for errors
    +-- Calls: SendInput for key/mouse emulation
    +-- Interacts with: Settings2 dialogs

Eye-trackers
    +-- BKBTobiiREX --> loads tobii_stream_engine.dll
    +-- BKBTET --> works via sockets (port 6555)
    +-- Both call: on_gaze_data() --> interacts with MagicWindow, CircleWindow

================================================================================
                            CONSTANTS
================================================================================

MH_NUM_SCANCODES        105     basic scan codes (103 + ЛКМ + ПКМ)
MH_NUM_SCANCODES_EXTRA  108     with extra operations
NUM_MAGIC_WINDOWS       24      number of magic windows

================================================================================
                            NEW FEATURES
================================================================================

MOUSE BUTTON CLICK SUPPORT (ЛКМ / ПКМ):
---------------------------------------
Added "ЛКМ" (Left Mouse Button) and "ПКМ" (Right Mouse Button) options
- Available in IDC_BUTTON6 and IDC_BUTTON6_1 dropdowns at positions 1-2
- Special scan codes: 0xE110 (ЛКМ), 0xE111 (ПКМ)
- Fully functional in all modes including autoclicker feature
- Works with both primary and secondary key assignments
- Implemented in MHKeypad::Press4 using SendInput with INPUT_MOUSE

Files Modified:
- Settings.h: Updated MH_NUM_SCANCODES from 103 to 105
- Settings.cpp: Added ЛКМ and ПКМ entries to dlg_scancodes array
- MHKeypad.cpp: Added mouse click handling in Press4 method

================================================================================
                              END OF DOCUMENT
================================================================================
