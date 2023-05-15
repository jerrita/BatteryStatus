#include <windows.h>
#include <Poclass.h>
#include <setupapi.h>
#include <Batclass.h>
#include <cstdio>
#include <powerbase.h>

// 刷新时间
#define DELAY 5000

BATTERY_STATUS GetBatteryState(void);