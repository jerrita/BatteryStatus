# BatteryStatus

这是一个小工具，用于查看实时电池放电速率以及剩余电量。

本工具为 C++ 编写，对电量消耗极小

## 使用方法

1. 打开 include/battery.h, 修改 DELAY 为你想要的延迟时间 (单位: 毫秒, 默认: 5000)
2. cmake -S . -B build
3. cmake --build build --config Release
4. 在 build/Release 文件夹中可以找到生成的工具
