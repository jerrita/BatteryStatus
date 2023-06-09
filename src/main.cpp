#include "battery.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HFONT hFont;       // 绘制文本所用的字体句柄
    static int width, height; // 窗口宽度和高度
    static COLORREF bgColors[] = {
        // 不同电池状态下的背景色
        RGB(255, 255, 255),    // 未知状态，白色背景
        RGB(255, 192, 203),    // 电源交流/充电，粉色背景
        RGB(0x66, 0xcc, 0xff), // 天依蓝
    };
    static TCHAR statusText[1024]; // 显示电池状态的字符串
    static int statusLen;          // 显示电池状态的字符串长度
    static BATTERY_STATUS bsPrev;  // 上一次查询的电池状态
    BATTERY_STATUS bs = {0};       // 当前查询的电池状态

    switch (msg)
    {
    case WM_CREATE:
    {
        // 创建字体
        LOGFONT lf = {};
        lf.lfWeight = FW_BOLD;
        lf.lfHeight = 24;
        hFont = CreateFontIndirect(&lf);

        // 初始化上一次查询的电池状态为空
        bsPrev.PowerState = 0;
        bsPrev.Capacity = 0;
        bsPrev.Voltage = 0;
        bsPrev.Rate = 0;

        // 启动 5 秒定时器
        SetTimer(hwnd, 1, DELAY, nullptr);
        break;
    }

    case WM_PAINT:
    {
        // // 获取电池状态, 但无法获取电池放电率
        // // 主要用于损耗程度检测
        // SYSTEM_POWER_STATUS sps;
        // if (GetSystemPowerStatus(&sps) == 0)
        // {
        //     // 获取失败，显示错误信息
        //     RECT rc = {};
        //     GetClientRect(hwnd, &rc);
        //     FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));
        //     SetTextColor(hdc, RGB(255, 0, 0));
        //     SetBkMode(hdc, TRANSPARENT);
        //     TCHAR text[] = TEXT("无法获取电池状态");
        //     DrawText(hdc, text, strlen(text), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        //     EndPaint(hwnd, &ps);
        //     return 0;
        // }

        bs = GetBatteryState();

        // 计算电池状态字符串
        if (bsPrev.PowerState == bs.PowerState && bsPrev.Capacity == bs.Capacity && bsPrev.Voltage == bs.Voltage && bsPrev.Rate == bs.Rate)
        {
            // 电池状态没有改变
            statusText[0] = '\0';
            statusLen = 0;
        }
        else
        {
            // 电池状态改变，重新计算状态字符串
            switch (bs.PowerState)
            {
            case 0:
                sprintf_s(statusText, TEXT("【未知状态】\nCapacity: %d mWh\nVoltage: %d mV\nRate: %d mW"), bs.Capacity, bs.Voltage, bs.Rate);
                break;
            case 1:
                sprintf_s(statusText, TEXT("【交流/充电】\nCapacity: %d mWh\nVoltage: %d mV\nRate: %d mW"), bs.Capacity, bs.Voltage, bs.Rate);
                break;
            case 2:
                sprintf_s(statusText, TEXT("【直流/放电】\nCapacity: %d mWh\nVoltage: %d mV\nRate: %d mW"), bs.Capacity, bs.Voltage, bs.Rate);
                break;
            }
            statusLen = strlen(statusText);
        }

        // 绘制电池状态字符串
        if (statusLen > 0)
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rc = {0, 0, width, height};
            FillRect(hdc, &rc, CreateSolidBrush(bgColors[bs.PowerState]));
            SetTextColor(hdc, RGB(0, 0, 0));
            SetBkMode(hdc, TRANSPARENT);
            SelectObject(hdc, hFont);
            DrawText(hdc, statusText, statusLen, &rc, DT_CENTER | DT_VCENTER);

            EndPaint(hwnd, &ps);
        }

        break;
    }

    case WM_SIZE:
    {
        width = LOWORD(lParam);
        height = HIWORD(lParam);
        InvalidateRect(hwnd, nullptr, FALSE);
        break;
    }

    case WM_TIMER:
    {
        if (wParam == 1)
        {
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        break;
    }

    case WM_DESTROY:
    {
        // 销毁窗口
        DeleteObject(hFont);
        PostQuitMessage(0);
        break;
    }

    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    bsPrev = bs;
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // 注册窗口类
    const wchar_t *className = L"BatteryStatusWindowClass";
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    if (!RegisterClassW(&wc))
    {
        MessageBoxW(nullptr, L"无法注册窗口类", L"错误", MB_OK | MB_ICONERROR);
        return -1;
    }

    // 创建窗口
    HWND hwnd = CreateWindowW(className, L"电池状态", WS_OVERLAPPED | WS_SYSMENU,
                              CW_USEDEFAULT, 0, 300, 150, nullptr, nullptr, hInstance, nullptr);
    if (!hwnd)
    {
        MessageBoxW(nullptr, L"无法创建窗口", L"错误", MB_OK | MB_ICONERROR);
        return -1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}
