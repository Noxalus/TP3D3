#include "stdafx.h"
#include "InputManager.h"


InputManager::InputManager(void)
{
	dInput = NULL;
	mouseDevice = NULL;
	keyboardDevice = NULL;

	// Zero Fill the keyPressState
	// used to record keypresses
	for (int i = 0; i < 256; i++){
		keyPressState[i] = 0;
	}
}

InputManager::~InputManager(void)
{
	if (dInput) 
    { 
        if (mouseDevice) 
        { 
            mouseDevice->Unacquire(); 
            mouseDevice->Release();
            mouseDevice = NULL; 
        }
		if (keyboardDevice){
			keyboardDevice->Unacquire(); 
            keyboardDevice->Release();
            keyboardDevice = NULL; 
		}
        dInput->Release();
        dInput = NULL; 
    } 
}

//
//   FUNCTION: init(HINSTANCE hInst, HWND wndHandle)
//
//   PURPOSE: Creates an Input Object for monitoring mouse input, and one for keyboard input.
//
bool InputManager::init(HINSTANCE hInst, HWND wndHandle)
{
	HRESULT hr;

	// Create a direct input object

    hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION, 
                            IID_IDirectInput8, (void**)&dInput, NULL); 

	if FAILED(hr){ 
		return FALSE;
	}

    // Create a device for monitoring the mouse
    if FAILED(dInput->CreateDevice(GUID_SysMouse, &mouseDevice, NULL))
		return FALSE; 
	if FAILED(mouseDevice->SetDataFormat(&c_dfDIMouse))
		return FALSE; 
    if FAILED(mouseDevice->SetCooperativeLevel(wndHandle, DISCL_FOREGROUND | DISCL_EXCLUSIVE))
        return FALSE; 
    if FAILED(mouseDevice->Acquire())
        return FALSE; 

	// Create a device for monitoring the keyboard
	if (FAILED(dInput->CreateDevice(GUID_SysKeyboard, &keyboardDevice, NULL)))
		return false;
	if (FAILED(keyboardDevice->SetDataFormat(&c_dfDIKeyboard)))
		return false;
	if (FAILED(keyboardDevice->SetCooperativeLevel(wndHandle, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)))
		return false;
	if (FAILED(keyboardDevice->Acquire()))
		return false;

	return true;
}


//
//   FUNCTION: getInput()
//
//   PURPOSE: Gets the state of the keyboard and mouse, if the device has been lost it trys to reaquire it
//
void InputManager::getInput()
{
	HRESULT hr;

	hr = mouseDevice->GetDeviceState(sizeof(DIMOUSESTATE),(LPVOID)&mouseState); 
	if (FAILED (hr))
	{
		// try and reacquire the input device
		mouseDevice->Acquire();
	}

	keyboardDevice->GetDeviceState(sizeof(UCHAR[256]), (LPVOID)keyState);
	if (FAILED (hr))
	{
		// try and reacquire the input device
		keyboardDevice->Acquire();
	}
}

//
//   FUNCTION: getMouseMovingX()
//
//   PURPOSE: returns the amount the mouse is moving in the X axis
//
int InputManager::getMouseMovingX() 
{
	return mouseState.lX; 
}

//
//   FUNCTION: getMouseMovingY()
//
//   PURPOSE: returns the amount the mouse is moving in the Y axis
//
int InputManager::getMouseMovingY() 
{
	return mouseState.lY; 
}

//
//   FUNCTION: isButtonDown(int which)
//
//   PURPOSE: Gets the state of the keyboard and mouse, if the device has been lost it trys to reaquire it
//
bool InputManager::isButtonDown(int button)
{
	//check the state of the button
	if (mouseState.rgbButtons[button] & 0x80){
		return true;
	} else {
		return false;
	}
}

//
//   FUNCTION: keyDown(DWORD key)
//
//   PURPOSE: returns if a key is currently pressed.
//
bool InputManager::keyDown(DWORD key)
{
	//check the state of the key
	if (keyState[key] & 0x80){
		return true;
	} else {
		return false;
	}
}

//
//   FUNCTION: keyUp(DWORD key)
//
//   PURPOSE: returns if a key is currently not pressed.
//
bool InputManager::keyUp(DWORD key)
{
	//check the state of the key
	if (keyState[key] & 0x80){
		return false;
	} else {
		return true;
	}
}

//
//   FUNCTION: keyPress(DWORD key)
//
//   PURPOSE: returns if a key has been pressed and then depressed
//
bool InputManager::keyPress(DWORD key)
{

	//check for keydown
	if (keyDown(key)){
		keyPressState[key] = 1;
	}
	//check for key reaching the keydown state
	if (keyPressState[key] == 1){
		//check for key release
		if (keyUp(key))
			keyPressState[key] = 2;
	}

	//check if key has been pressed and released
	if (keyPressState[key] == 2){
		//reset the key status
		keyPressState[key] = 0;
		return true;
	}
	
	return false;
}