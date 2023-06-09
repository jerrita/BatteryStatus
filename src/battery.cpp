#include "battery.h"

DEFINE_GUID(GUID_DEVCLASS_BATTERY, 0x72631E54, 0x78A4, 0x11D0, 0xBC, 0xF7, 0x00, 0xAA, 0x00, 0xB7, 0xB3, 0x2A);

#pragma comment(lib, "SetupAPI.lib")
#pragma comment(lib, "PowrProf.lib")

BATTERY_STATUS GetBatteryState2(void)
{
    BATTERY_STATUS batteryStatus = {0};
    HDEVINFO hdev = ::SetupDiGetClassDevs(&GUID_DEVCLASS_BATTERY, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (INVALID_HANDLE_VALUE != hdev)
    {
        // 搜索限制为最多 10 块电池
        for (int idev = 0; idev < 10; idev++)
        {
            SP_DEVICE_INTERFACE_DATA did = {0};
            did.cbSize = sizeof(did);

            if (::SetupDiEnumDeviceInterfaces(hdev, 0, &GUID_DEVCLASS_BATTERY, idev, &did))
            {
                DWORD cbRequired = 0;

                ::SetupDiGetDeviceInterfaceDetail(hdev, &did, 0, 0, &cbRequired, 0);
                if (ERROR_INSUFFICIENT_BUFFER == ::GetLastError())
                {
                    PSP_DEVICE_INTERFACE_DETAIL_DATA pdidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA)::LocalAlloc(LPTR, cbRequired);
                    if (pdidd)
                    {
                        pdidd->cbSize = sizeof(*pdidd);
                        if (::SetupDiGetDeviceInterfaceDetail(hdev, &did, pdidd, cbRequired, &cbRequired, 0))
                        {
                            // 枚举电池, 索取信息
                            HANDLE hBattery = ::CreateFile(pdidd->DevicePath,
                                                           GENERIC_READ | GENERIC_WRITE,
                                                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                           NULL,
                                                           OPEN_EXISTING,
                                                           FILE_ATTRIBUTE_NORMAL,
                                                           NULL);
                            if (INVALID_HANDLE_VALUE != hBattery)
                            {
                                // 向电池索取标签.
                                BATTERY_QUERY_INFORMATION bqi = {0};

                                DWORD dwWait = 0;
                                DWORD dwOut;

                                if (::DeviceIoControl(hBattery,
                                                      IOCTL_BATTERY_QUERY_TAG,
                                                      &dwWait,
                                                      sizeof(dwWait),
                                                      &bqi.BatteryTag,
                                                      sizeof(bqi.BatteryTag),
                                                      &dwOut,
                                                      NULL) &&
                                    bqi.BatteryTag)
                                {
                                    // 查询电池状态
                                    BATTERY_WAIT_STATUS bws = {0};
                                    bws.BatteryTag = bqi.BatteryTag;
                                    BATTERY_STATUS bs = {0};

                                    if (::DeviceIoControl(hBattery,
                                                          IOCTL_BATTERY_QUERY_STATUS,
                                                          &bws,
                                                          sizeof(bws),
                                                          &bs,
                                                          sizeof(bs),
                                                          &dwOut,
                                                          NULL))
                                    {
                                        batteryStatus.Capacity = bs.Capacity;
                                        batteryStatus.PowerState = bs.PowerState;
                                        batteryStatus.Voltage = bs.Voltage;
                                        batteryStatus.Rate = bs.Rate;
                                    }
                                }
                                ::CloseHandle(hBattery);
                            }
                        }
                        ::LocalFree(pdidd);
                    }
                }
            }
            else if (ERROR_NO_MORE_ITEMS == ::GetLastError())
            {
                break;
            }
        }
        ::SetupDiDestroyDeviceInfoList(hdev);
    }

    return batteryStatus;
}

BATTERY_STATUS GetBatteryState(void)
{
    // store the time of the last call
    static DWORD lastCall = 0;
    static BATTERY_STATUS lastBatteryStatus = {0};

    // Get current time
    DWORD now = GetTickCount();

    // If the last call was less than 5 seconds ago, return the previous result
    if (now - lastCall < DELAY)
        return lastBatteryStatus;

    // Otherwise, update the last call time
    lastCall = now;
    return lastBatteryStatus = GetBatteryState2();
}

int test()
{
    LONG ntStatus = S_OK;
    SYSTEM_BATTERY_STATE pi = {0};

    ntStatus = CallNtPowerInformation(SystemBatteryState, NULL, 0, &pi, sizeof(pi));
    if (0 == ntStatus)
    {
        printf("Battery: %lu / %lu\n", pi.RemainingCapacity, pi.MaxCapacity);
    }

    BATTERY_STATUS bs = GetBatteryState();
    printf("Capacity: %lu mWh PowerState: %lu Voltage: %lu mv Rate: %ld mWh\n", bs.Capacity, bs.PowerState, bs.Voltage, bs.Rate);
    return 0;
}