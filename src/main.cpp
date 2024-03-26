#include "windows.h"

bool destroyed = false;

LRESULT MainWindowCallback(
  HWND window,
  UINT message,
  WPARAM wparam,
  LPARAM lparam
)
{
    LRESULT result = 0;
    switch(message){
        case WM_SIZE:{

        } break;
        case WM_DESTROY:{
            destroyed = true;
            result = DefWindowProc(window, message, wparam, lparam);
        } break;
        case WM_CLOSE:{
            result = DefWindowProc(window, message, wparam, lparam);
        }break;
        case WM_ACTIVATEAPP:{

        }break;
        case WM_PAINT:{
            PAINTSTRUCT paint;
            HDC deviceContext = BeginPaint(window, &paint);
            int x = paint.rcPaint.left;
            int y = paint.rcPaint.top;
            int height = paint.rcPaint.bottom - paint.rcPaint.top;
            int width = paint.rcPaint.right - paint.rcPaint.left;
            PatBlt(deviceContext, x, y, width, height, WHITENESS);
            EndPaint(window, &paint);
        }break;
        default:{
            result = DefWindowProc(window, message, wparam, lparam);
        }break;
    }
    return result;
}

int WinMain(
  HINSTANCE instance,
  HINSTANCE prevInstance,
  LPSTR     commandLine,
  int       showCode
){
    WNDCLASS windowClass = {};
    windowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    windowClass.lpfnWndProc = MainWindowCallback;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = "WindowClass";

    RegisterClass(&windowClass);

    HWND windowHandle = CreateWindowEx(
        0,
        windowClass.lpszClassName, 
        "Window",
        WS_OVERLAPPEDWINDOW|WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        instance,
        0);

    MSG message;
    while(GetMessage(&message, 0, 0, 0) > 0){
        if(destroyed){
            return 0;
        }
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    return 0;
}
