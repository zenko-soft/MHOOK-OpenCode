#include "Gamepad.h"
#include "Settings.h"
bool MHGamepad::enabled = false;
DWORD MHGamepad::prevButtons[4] = {0};
int MHGamepad::deadzoneX = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
int MHGamepad::deadzoneY = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
int MHGamepad::sensitivity = 3;
bool MHGamepad::Initialize()
{
    if (!MHSettings::flag_gamepad_enabled) return false;
    sensitivity = MHSettings::gamepad_sensitivity;
    for (DWORD i = 0; i < 4; i++)
    {
        XINPUT_STATE state;
        if (XInputGetState(i, &state) == ERROR_SUCCESS)
        {
            enabled = true;
            return true;
        }
    }
    return false;
}
void MHGamepad::Update()
{
    if (!enabled) return;
    sensitivity = MHSettings::gamepad_sensitivity;
    for (DWORD i = 0; i < 4; i++)
    {
        XINPUT_STATE state;
        if (XInputGetState(i, &state) == ERROR_SUCCESS)
        {
            HandleGamepad(i, &state);
        }
    }
}
void MHGamepad::Shutdown()
{
    enabled = false;
}
bool MHGamepad::IsConnected(DWORD userIndex)
{
    if (userIndex >= 4) return false;
    XINPUT_STATE state;
    return XInputGetState(userIndex, &state) == ERROR_SUCCESS;
}
void MHGamepad::SetEnabled(bool en)
{
    enabled = en;
}
bool MHGamepad::IsEnabled()
{
    return enabled;
}
void MHGamepad::HandleGamepad(DWORD userIndex, XINPUT_STATE* state)
{
    short stickX = state->Gamepad.sThumbLX;
    short stickY = state->Gamepad.sThumbLY;
    if (stickX > deadzoneX || stickX < -deadzoneX ||
        stickY > deadzoneY || stickY < -deadzoneY)
    {
        SimulateMouseMove(stickX, stickY);
    }
    WORD buttons = state->Gamepad.wButtons;
    HandleButton(userIndex, XINPUT_GAMEPAD_A, (buttons & XINPUT_GAMEPAD_A) != 0);
    HandleButton(userIndex, XINPUT_GAMEPAD_B, (buttons & XINPUT_GAMEPAD_B) != 0);
    HandleButton(userIndex, XINPUT_GAMEPAD_X, (buttons & XINPUT_GAMEPAD_X) != 0);
    HandleButton(userIndex, XINPUT_GAMEPAD_Y, (buttons & XINPUT_GAMEPAD_Y) != 0);
    HandleButton(userIndex, XINPUT_GAMEPAD_LEFT_SHOULDER, (buttons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0);
    HandleButton(userIndex, XINPUT_GAMEPAD_RIGHT_SHOULDER, (buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0);
    HandleButton(userIndex, XINPUT_GAMEPAD_LEFT_THUMB, (buttons & XINPUT_GAMEPAD_LEFT_THUMB) != 0);
    HandleButton(userIndex, XINPUT_GAMEPAD_RIGHT_THUMB, (buttons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0);
    HandleButton(userIndex, XINPUT_GAMEPAD_START, (buttons & XINPUT_GAMEPAD_START) != 0);
    HandleButton(userIndex, XINPUT_GAMEPAD_BACK, (buttons & XINPUT_GAMEPAD_BACK) != 0);
    HandleButton(userIndex, XINPUT_GAMEPAD_DPAD_UP, (buttons & XINPUT_GAMEPAD_DPAD_UP) != 0);
    HandleButton(userIndex, XINPUT_GAMEPAD_DPAD_DOWN, (buttons & XINPUT_GAMEPAD_DPAD_DOWN) != 0);
    HandleButton(userIndex, XINPUT_GAMEPAD_DPAD_LEFT, (buttons & XINPUT_GAMEPAD_DPAD_LEFT) != 0);
    HandleButton(userIndex, XINPUT_GAMEPAD_DPAD_RIGHT, (buttons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0);
    prevButtons[userIndex] = buttons;
}
void MHGamepad::SimulateMouseMove(short stickX, short stickY)
{
    double moveX = (stickX / 32767.0) * sensitivity;
    double moveY = (stickY / 32767.0) * sensitivity;
    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    input.mi.dx = (LONG)moveX;
    input.mi.dy = (LONG)moveY;
    SendInput(1, &input, sizeof(INPUT));
}
void MHGamepad::HandleButton(DWORD userIndex, WORD button, bool pressed)
{
    bool wasPressed = (prevButtons[userIndex] & button) != 0;
    if (pressed && !wasPressed)
    {
        DWORD mouseButton = 0;
        switch (button)
        {
            case XINPUT_GAMEPAD_A:
            case XINPUT_GAMEPAD_DPAD_DOWN:
                mouseButton = MOUSEEVENTF_LEFTDOWN;
                break;
            case XINPUT_GAMEPAD_B:
            case XINPUT_GAMEPAD_DPAD_RIGHT:
                mouseButton = MOUSEEVENTF_RIGHTDOWN;
                break;
            case XINPUT_GAMEPAD_X:
            case XINPUT_GAMEPAD_DPAD_LEFT:
                mouseButton = MOUSEEVENTF_MIDDLEDOWN;
                break;
            case XINPUT_GAMEPAD_Y:
            case XINPUT_GAMEPAD_DPAD_UP:
                mouseButton = MOUSEEVENTF_WHEEL;
                break;
        }
        if (mouseButton)
        {
            INPUT input = {0};
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = mouseButton;
            SendInput(1, &input, sizeof(INPUT));
        }
    }
    else if (!pressed && wasPressed)
    {
        DWORD mouseButton = 0;
        switch (button)
        {
            case XINPUT_GAMEPAD_A:
            case XINPUT_GAMEPAD_DPAD_DOWN:
                mouseButton = MOUSEEVENTF_LEFTUP;
                break;
            case XINPUT_GAMEPAD_B:
            case XINPUT_GAMEPAD_DPAD_RIGHT:
                mouseButton = MOUSEEVENTF_RIGHTUP;
                break;
            case XINPUT_GAMEPAD_X:
            case XINPUT_GAMEPAD_DPAD_LEFT:
                mouseButton = MOUSEEVENTF_MIDDLEUP;
                break;
        }
        if (mouseButton)
        {
            INPUT input = {0};
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = mouseButton;
            SendInput(1, &input, sizeof(INPUT));
        }
    }
}