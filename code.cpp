// Direct2D.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Direct2D.h"

#include <d2d1.h>                 // *** DIRECT2D ADDED ***
#pragma comment(lib, "d2d1.lib")  // *** DIRECT2D ADDED ***
#define MAX_LOADSTRING 100
#include <vector>
#include <string>


// your code
#include <wincodec.h>   // WIC

#pragma comment(lib, "windowscodecs.lib")

IWICImagingFactory* g_wicFactory = nullptr;
ID2D1Bitmap* g_myImage = nullptr;



HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

// *** DIRECT2D ADDED: Global graphics objects ***
ID2D1Factory* pFactory = nullptr;
ID2D1HwndRenderTarget* pRenderTarget = nullptr;
ID2D1SolidColorBrush* pBrush = nullptr;







struct uiRect
{
    std::string name;   // <-- ADD THIS

    float left, top, right, bottom;
    D2D1_COLOR_F color;

    D2D1_RECT_F getRectF() const
    {
        return D2D1::RectF(left, top, right, bottom);
    }

    bool contains(float px, float py) const
    {
        return px >= left && px <= right &&
            py >= top && py <= bottom;
    }

    float width()  const { return right - left; }
    float height() const { return bottom - top; }

    void move(float dx, float dy)
    {
        left += dx; right += dx;
        top += dy;  bottom += dy;
    }
};



struct MyRect
{
    float left, top, right, bottom;   // Direct2D style
    D2D1_COLOR_F color;

    // Helper to get D2D rectangle (Direct2D wants left, top, right, bottom)
    D2D1_RECT_F getRectF() const
    {
        return D2D1::RectF(left, top, right, bottom);
    }

    // Check if point is inside
    bool contains(float px, float py) const
    {
        return px >= left && px <= right &&
            py >= top && py <= bottom;
    }

    // Width & height helper
    float width() const { return right - left; }
    float height() const { return bottom - top; }

    // Move rectangle by dx, dy
    void move(float dx, float dy)
    {
        left += dx; right += dx;
        top += dy; bottom += dy;
    }
};



std::vector<uiRect> UI_rectangles;


// In your globals
std::vector<MyRect> rectangles;



std::vector<float> ReclastPos;


// Dragging state
int draggingIndex = -1;
int left_Resize_Index = -1;
int Right_Resize_Index = -1;

float dragOffsetX = 0;
float dragOffsetY = 0;

float Rect_resize_mStartPosX = 0;










// Bitmap
        // your code bitmap Image
ID2D1Bitmap* LoadBitmapFromFile(
    ID2D1RenderTarget* rt,
    PCWSTR uri)
{
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* converter = nullptr;
    ID2D1Bitmap* bitmap = nullptr;

    g_wicFactory->CreateDecoderFromFilename(
        uri, nullptr, GENERIC_READ,
        WICDecodeMetadataCacheOnLoad, &decoder);

    decoder->GetFrame(0, &frame);

    g_wicFactory->CreateFormatConverter(&converter);
    converter->Initialize(
        frame,
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        nullptr, 0.0,
        WICBitmapPaletteTypeMedianCut);

    rt->CreateBitmapFromWicBitmap(converter, nullptr, &bitmap);

    if (decoder) decoder->Release();
    if (frame) frame->Release();
    if (converter) converter->Release();

    return bitmap;
}







// Forward declarations
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// Image Bitmap
int APIENTRY wWinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR    lpCmdLine,
    int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // --- Initialize COM for WIC ---
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) return 0;

    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&g_wicFactory)
    );
    if (FAILED(hr)) return 0;

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DIRECT2D, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
        return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DIRECT2D));

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // --- Cleanup WIC ---
    if (g_wicFactory) g_wicFactory->Release();
    CoUninitialize();

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DIRECT2D));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_DIRECT2D);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
        return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

        // *** DIRECT2D ADDED: Initialize Direct2D when window is created ***
    case WM_CREATE:
    {
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);

        RECT rc;
        GetClientRect(hWnd, &rc);

        pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(
                hWnd,
                D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)
            ),
            &pRenderTarget
        );

        pRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::DeepSkyBlue),
            &pBrush
        );



        // --- Initialize rectangles here ---
        rectangles.push_back({ 800, 100, 1000, 175, D2D1::ColorF(D2D1::ColorF::Red) });
        rectangles.push_back({ 100, 100, 300, 175, D2D1::ColorF(D2D1::ColorF::DeepSkyBlue) });




        // --- UI rectangles ---//

// ===== MAIN =====
        UI_rectangles.push_back({ "Main background", 0,0 - 40,1920,1080 - 40, D2D1::ColorF(0x3F2F49) });
    }
    break;
 

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    // *** DIRECT2D MODIFIED: Drawing with GPU instead of GDI ***
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);

        pRenderTarget->BeginDraw();
        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

        // your code bitmap Image
        if (!g_myImage)
        {
            g_myImage = LoadBitmapFromFile(pRenderTarget, L"image.png");
        }


        for (auto& r : UI_rectangles)
        {
            pBrush->SetColor(r.color);
            pRenderTarget->FillRectangle(r.getRectF(), pBrush);
        }

        for (auto& r : rectangles)
        {
            pBrush->SetColor(r.color);
            pRenderTarget->FillRectangle(r.getRectF(), pBrush);
        }

        // your code bitmap Image
        D2D1_RECT_F rect = D2D1::RectF(100, 100, 500, 350);
        pRenderTarget->DrawBitmap(g_myImage, rect);


        pRenderTarget->EndDraw();
        EndPaint(hWnd, &ps);



    }
    break;

    case WM_SIZE:
    {
        if (pRenderTarget)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);

            D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
            pRenderTarget->Resize(size);
        }
    }
    break;

    // *** DIRECT2D MODIFIED: Release GPU resources ***
    case WM_DESTROY:
        if (pBrush) pBrush->Release();
        if (pRenderTarget) pRenderTarget->Release();
        if (pFactory) pFactory->Release();
        PostQuitMessage(0);

        // your code bitmap Image
        if (g_myImage) g_myImage->Release();
        if (g_wicFactory) g_wicFactory->Release();
        CoUninitialize();

        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
