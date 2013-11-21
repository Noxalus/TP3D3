#pragma once
class IInputNotifier
{
public:
   IInputNotifier() {}
   virtual ~IInputNotifier() {}

   virtual void OnKeyDown(WPARAM wParam, LPARAM lParam) = 0;
   virtual void OnKeyUp(WPARAM wParam, LPARAM lParam) = 0;
};

