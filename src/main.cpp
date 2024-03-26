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

static bool running;

struct OffscreenBuffer{
    const int bytesPerPixel = 4;
    BITMAPINFO info;
    void* memory;
    int width;
    int height;
    int pitch;
}; 

struct WindowDimensions{
    int width;
    int height;
};

WindowDimensions GetWindowDimensions(HWND window){
    WindowDimensions dimensions = {};
    RECT clientRect;
    GetClientRect(window, &clientRect);
    dimensions.width = clientRect.right - clientRect.left;
    dimensions.height = clientRect.bottom - clientRect.top;
    return dimensions;
}

static OffscreenBuffer offscreenBuffer;

void RenderWeirdGradient(OffscreenBuffer buffer, int xOffset, int yOffset){
    uint8* row = (uint8*)buffer.memory;
    for(int y=0;y<buffer.height;y++){
        uint32* pixel = (uint32*)row;
        for(int x=0;x<buffer.width;x++){
            uint8 blue = x+xOffset;
            uint8 green = y+yOffset;
            *pixel++ = ((green << 8) | blue);
        }
        row+=buffer.pitch;
    }
}

void ResizeDIBSection(OffscreenBuffer *buffer, int width, int height){
    if(buffer->memory){
        VirtualFree(buffer->memory, 0, MEM_RELEASE);   
    }
    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = width;
    buffer->info.bmiHeader.biHeight = -height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;
    buffer->width = width;
    buffer->height = height;
    int bitmapMemorySize = width*height*buffer->bytesPerPixel;
    buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    buffer->pitch = width*buffer->bytesPerPixel;
}

void CopyBufferToWindow(OffscreenBuffer buffer, HDC deviceContext, int clientWidth, int clientHeight, int x, int y, int width, int height){
    StretchDIBits(deviceContext, 0, 0, clientWidth, clientHeight, 0, 0, buffer.width, buffer.height, buffer.memory, &buffer.info, DIB_RGB_COLORS, SRCCOPY);
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
            
            WindowDimensions dimensions = GetWindowDimensions(window);
            CopyBufferToWindow(offscreenBuffer, deviceContext, dimensions.width, dimensions.height ,x,y,width,height);
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
    windowClass.style = CS_HREDRAW|CS_VREDRAW;
    windowClass.lpfnWndProc = MainWindowCallback;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = "WindowClass";

    ResizeDIBSection(&offscreenBuffer, 1280, 720);
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
        RenderWeirdGradient(offscreenBuffer, xOffset, yOffset);
        WindowDimensions dimensions = GetWindowDimensions(window);
        CopyBufferToWindow(offscreenBuffer,deviceContext,dimensions.width,dimensions.height,0,0,dimensions.width,dimensions.height);
        ReleaseDC(window, deviceContext);

        xOffset++;
        yOffset++;
    }

    return 0;
}
