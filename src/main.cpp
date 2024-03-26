#include "windows.h"
#include <stdint.h>
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

const int bytesPerPixel = 4;
static bool running;
static BITMAPINFO bitmapInfo;
static void* bitmapMemory;
static int bitmapWidth;
static int bitmapHeight;

void RenderWeirdGradient(int xOffset, int yOffset){
    int pitch = bitmapWidth*bytesPerPixel;
    uint8* row = (uint8*)bitmapMemory;
    for(int y=0;y<bitmapHeight;y++){
        uint32* pixel = (uint32*)row;
        for(int x=0;x<bitmapWidth;x++){
            uint8 blue = x+xOffset;
            uint8 green = y+yOffset;
            *pixel++ = ((green << 8) | blue);
        }
        row+=pitch;
    }
}

void ResizeDIBSection(int width, int height){
    if(bitmapMemory){
        VirtualFree(bitmapMemory, 0, MEM_RELEASE);   
    }
    bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
    bitmapInfo.bmiHeader.biWidth = width;
    bitmapInfo.bmiHeader.biHeight = -height;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;
    bitmapWidth = width;
    bitmapHeight = height;
    int bitmapMemorySize = width*height*bytesPerPixel;
    bitmapMemory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

void _UpdateWindow(HDC deviceContext, RECT* clientRect, int x, int y, int width, int height){
    //StretchDIBits(deviceContext, x, y, width, height, x, y, width, height, bitmapMemory, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
    int windowWidth = clientRect->right - clientRect->left;
    int windowHeight = clientRect->bottom - clientRect->top;
    StretchDIBits(deviceContext, 0, 0, bitmapWidth, bitmapHeight, 0, 0, windowWidth, windowHeight, bitmapMemory, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

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
            RECT clientRect;
            GetClientRect(window, &clientRect);
            int width = clientRect.right - clientRect.left;
            int height = clientRect.bottom - clientRect.top;
            ResizeDIBSection(width, height);
        } break;
        case WM_DESTROY:{
            running = false;
            result = DefWindowProc(window, message, wparam, lparam);
        } break;
        case WM_CLOSE:{
            running = false;
            result = DefWindowProc(window, message, wparam, lparam);
        }break;
        case WM_ACTIVATEAPP:{

        }break;
        case WM_PAINT:{
            PAINTSTRUCT paint;
            HDC deviceContext = BeginPaint(window, &paint);
            int x = paint.rcPaint.left;
            int y = paint.rcPaint.top;
            int width = paint.rcPaint.right - paint.rcPaint.left;
            int height = paint.rcPaint.bottom - paint.rcPaint.top;
            RECT clientRect;
            GetClientRect(window, &clientRect);
            _UpdateWindow(deviceContext,&clientRect,x,y,width,height);
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

    HWND window = CreateWindowEx(
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

    running = true;
    int xOffset = 0;
    int yOffset = 0;
    while(running){
        MSG message;
        while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)){
            if(message.message == WM_QUIT){
                running = false;
            }
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        HDC deviceContext = GetDC(window);
        RenderWeirdGradient(xOffset, yOffset);
        RECT clientRect;
        GetClientRect(window, &clientRect);
        int width = clientRect.right - clientRect.left;
        int height = clientRect.bottom - clientRect.top;
        _UpdateWindow(deviceContext,&clientRect,0,0,width,height);
        ReleaseDC(window, deviceContext);

        xOffset++;
    }

    return 0;
}
