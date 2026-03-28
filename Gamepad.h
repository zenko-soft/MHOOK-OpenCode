#pragma once
#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "XInput.lib")
class MHGamepad
{
public:
    static bool Initialize();
    static void Update();
    static void Shutdown();
    static bool IsConnected(DWORD userIndex);
    static void SetEnabled(bool enabled);
    static bool IsEnabled();
private:
    static bool enabled;
    static DWORD prevButtons[4];
    static int deadzoneX;
    static int deadzoneY;
    static int sensitivity;
    static void HandleGamepad(DWORD userIndex, XINPUT_STATE* state);
    static void SimulateMouseMove(short stickX, short stickY);
    static void HandleButton(DWORD userIndex, WORD button, bool pressed);
};