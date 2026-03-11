// Direct2D.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Direct2D.h"

#include <d2d1.h>                 // *** DIRECT2D ADDED ***
#pragma comment(lib, "d2d1.lib")  // *** DIRECT2D ADDED ***
#define MAX_LOADSTRING 100
#include <vector>
#include <string>


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


// Forward declarations
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR    lpCmdLine,
    int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

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

        // ===== LEFT NAV =====
        UI_rectangles.push_back({ "Nav Icon", 15,10,65,60, D2D1::ColorF(D2D1::ColorF::White) });
        UI_rectangles.push_back({ "Left icon 1", 15,110,65,160, D2D1::ColorF(D2D1::ColorF::White) });
        UI_rectangles.push_back({ "Left icon 2", 15,210,65,260, D2D1::ColorF(D2D1::ColorF::White) });
        UI_rectangles.push_back({ "Left icon 3", 15,310,65,360, D2D1::ColorF(D2D1::ColorF::White) });

        // ===== MEDIA =====
        UI_rectangles.push_back({ "Import media", 102,110,317,185, D2D1::ColorF(0x8A2FFF) });

        UI_rectangles.push_back({ "Left media sort", 102,200,177,225, D2D1::ColorF(0x7635C1) });
        UI_rectangles.push_back({ "Right media sort", 218,200,293,225, D2D1::ColorF(0x7635C1) });

        UI_rectangles.push_back({ "Left media video Placeholder row 1", 102,250,202,350, D2D1::ColorF(0x2F2433) });
        UI_rectangles.push_back({ "Right media video Placeholder row 1", 218,250,318,350, D2D1::ColorF(0x2F2433) });

        UI_rectangles.push_back({ "Left media video Placeholder row 2", 102,400,202,500, D2D1::ColorF(0x2F2433) });
        UI_rectangles.push_back({ "Right media video Placeholder row 2", 218,400,318,500, D2D1::ColorF(0x2F2433) });

        UI_rectangles.push_back({ "Left media video Placeholder row 3", 102,550,202,650, D2D1::ColorF(0x2F2433) });
        UI_rectangles.push_back({ "Right media video Placeholder row 3", 218,550,318,650, D2D1::ColorF(0x2F2433) });

        // ===== PREVIEW + TIMELINE =====
        UI_rectangles.push_back({ "Preview + Timeline background", 345,90,1595,1040, D2D1::ColorF(0x483B51) });
        UI_rectangles.push_back({ "PreviewPanel", 553,100,1386,565, D2D1::ColorF(D2D1::ColorF::Black) });

        // Preview buttons left
        UI_rectangles.push_back({ "Go back button", 360,582,395,617, D2D1::ColorF(D2D1::ColorF::White) });
        UI_rectangles.push_back({ "Go Forward button", 426,582,461,617, D2D1::ColorF(D2D1::ColorF::White) });

        // Preview buttons middle
        UI_rectangles.push_back({ "Play button", 872,582,907,617, D2D1::ColorF(D2D1::ColorF::White) });
        UI_rectangles.push_back({ "Current Time", 931,582,966,617, D2D1::ColorF(D2D1::ColorF::White) });
        UI_rectangles.push_back({ "Total Time", 990,582,1025,617, D2D1::ColorF(D2D1::ColorF::White) });

        // Preview buttons right
        UI_rectangles.push_back({ "Zoom In", 1460,582,1495,617, D2D1::ColorF(D2D1::ColorF::White) });
        UI_rectangles.push_back({ "Zoom Out", 1530,582,1565,617, D2D1::ColorF(D2D1::ColorF::White) });

        // ===== TIMELINE =====
        UI_rectangles.push_back({ "TimelineBar", 360,637,1573,662, D2D1::ColorF(D2D1::ColorF::Red) });


        // ===== EFFECT PANEL =====
        UI_rectangles.push_back({ "Effect background", 1640,110,1840,710, D2D1::ColorF(0x3F3443) });

        UI_rectangles.push_back({ "Right Icon 1", 1855,110,1905,160, D2D1::ColorF(D2D1::ColorF::White) });
        UI_rectangles.push_back({ "Right Icon 2", 1855,210,1905,260, D2D1::ColorF(D2D1::ColorF::White) });
        UI_rectangles.push_back({ "Right Icon 3", 1855,310,1905,360, D2D1::ColorF(D2D1::ColorF::White) });

        UI_rectangles.push_back({ "Left effect video Placeholder row 1", 1640,125,1740,225, D2D1::ColorF(0x2F2433) });
        UI_rectangles.push_back({ "Right effect video Placeholder row 1", 1740,125,1840,225, D2D1::ColorF(0x2F2433) });

        UI_rectangles.push_back({ "Effect video Placeholder row 2", 1640,235,1740,335, D2D1::ColorF(0x2F2433) });
        UI_rectangles.push_back({ "Right effect video Placeholder row 2", 1740,235,1840,335, D2D1::ColorF(0x2F2433) });

        UI_rectangles.push_back({ "Left effect video Placeholder row 3", 1640,345,1740,445, D2D1::ColorF(0x2F2433) });
        UI_rectangles.push_back({ "Right effect video Placeholder row 3", 1740,345,1840,445, D2D1::ColorF(0x2F2433) });
    }
    break;
    case WM_LBUTTONDOWN:
    {
        int mx = LOWORD(lParam);
        int my = HIWORD(lParam);

        // Loop top-most first
        for (int i = rectangles.size() - 1; i >= 0; --i)
        {
            if (!rectangles[i].contains(mx, my))
                continue;

            // Left resize
            if (mx - rectangles[i].left < 15)
            {
                left_Resize_Index = i;
                Rect_resize_mStartPosX = mx;
                SetCapture(hWnd);
                break;
            }
            // Right resize
            else if (rectangles[i].right - mx < 15)
            {
                Right_Resize_Index = i;
                Rect_resize_mStartPosX = mx;
                SetCapture(hWnd);
                break;
            }
            // Dragging
            else
            {
                draggingIndex = i;
                dragOffsetX = mx - rectangles[i].left;
                dragOffsetY = my - rectangles[i].top;

                // Save last position for snap-back
                ReclastPos.clear();
                ReclastPos.push_back(rectangles[i].left);
                ReclastPos.push_back(rectangles[i].top);
                ReclastPos.push_back(rectangles[i].right);
                ReclastPos.push_back(rectangles[i].bottom);

                SetCapture(hWnd);
                break;
            }
        }
    }
    break;

    case WM_MOUSEMOVE:
    {
        int mx = LOWORD(lParam);
        int my = HIWORD(lParam);

        // Left resize
        if (left_Resize_Index != -1)
        {
            float dx = mx - Rect_resize_mStartPosX;
            rectangles[left_Resize_Index].left += dx;
            Rect_resize_mStartPosX = mx; // update start for next move
            InvalidateRect(hWnd, nullptr, FALSE);
        }
        // Right resize
        else if (Right_Resize_Index != -1)
        {
            float dx = mx - Rect_resize_mStartPosX;
            rectangles[Right_Resize_Index].right += dx; // ADD dx, not subtract
            Rect_resize_mStartPosX = mx;
            InvalidateRect(hWnd, nullptr, FALSE);
        }

        else if (draggingIndex != -1)
        {

            float dx = mx - dragOffsetX - rectangles[draggingIndex].left;
            float dy = my - dragOffsetY - rectangles[draggingIndex].top;

            // Move normally
            rectangles[draggingIndex].move(dx, dy);

            // Snap to right edge of other rectangles
            for (int i = 0; i < rectangles.size(); i++)
            {
                if (i == draggingIndex) continue;

                if (std::abs(rectangles[draggingIndex].left - rectangles[i].right) < 25.0f &&
                    std::abs(rectangles[draggingIndex].top - rectangles[i].top) < 100.0f)
                {
                    // Snap left edge to other's right edge
                    float w = rectangles[draggingIndex].width();
                    rectangles[draggingIndex].left = rectangles[i].right;
                    rectangles[draggingIndex].right = rectangles[draggingIndex].left + w;

                    // Align Y
                    float h = rectangles[draggingIndex].height();
                    rectangles[draggingIndex].top = rectangles[i].top;
                    rectangles[draggingIndex].bottom = rectangles[draggingIndex].top + h;
                    break;
                }
            }

            InvalidateRect(hWnd, nullptr, FALSE);
        }
    }
    break;

    case WM_LBUTTONUP:
    {
        left_Resize_Index = -1;
        Right_Resize_Index = -1;

        if (draggingIndex != -1)
        {
            int mx = LOWORD(lParam);
            int my = HIWORD(lParam);

            for (int i = 0; i < rectangles.size(); i++)
            {
                if (i == draggingIndex) continue;

                if (rectangles[i].contains((float)mx, (float)my))
                {
                    // Snap back to last position if dropped on top of another
                    if (ReclastPos.size() >= 4)
                    {


                        rectangles[draggingIndex].left = ReclastPos[0];
                        rectangles[draggingIndex].top = ReclastPos[1];
                        rectangles[draggingIndex].right = ReclastPos[2];
                        rectangles[draggingIndex].bottom = ReclastPos[3];
                    }
                    break;
                }
            }

            draggingIndex = -1;
            ReleaseCapture();
            InvalidateRect(hWnd, nullptr, FALSE);
        }

    }
    break;

    case WM_KEYDOWN: // dynamically create new rectangles
    {
        switch (wParam)
        {
        case 'N':  // Press 'N' to add a new rectangle
        {
            // Add a new rectangle at (50,50), size 100x100, green
            rectangles.push_back({ 50, 50, 100, 100, D2D1::ColorF(D2D1::ColorF::LimeGreen) });

            // Force repaint
            InvalidateRect(hWnd, nullptr, FALSE);
        }
        break;
        }
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
