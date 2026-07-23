#include "Fan.h"

#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

namespace {
int failures = 0;

void expect(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}
} // namespace

int main()
{
    Fan fan;

    expect(fan.IsFanOn(), "fan starts on");
    expect(fan.GetBladeCount() == 4, "fan starts with the core four-blade layout");
    fan.CycleBladeCount();
    expect(fan.GetBladeCount() == 5, "blade count cycles from four to five");
    fan.CycleBladeCount();
    expect(fan.GetBladeCount() == 3, "blade count wraps from five to three");
    fan.CycleBladeCount();
    expect(fan.GetBladeCount() == 4, "blade count cycles back to four");

    fan.TouchEventDown(200.0f, 200.0f);
    std::this_thread::sleep_for(std::chrono::milliseconds(620));
    fan.TouchEventRelease(200.0f, 200.0f);
    expect(fan.GetBladeCount() == 5, "a long press cycles the runtime blade count");
    expect(fan.IsFanOn(), "a long press does not toggle the fan");
    fan.CycleBladeCount();
    fan.CycleBladeCount();
    expect(fan.GetBladeCount() == 4, "test restores the default four-blade layout");

    fan.TouchEventDown(100.0f, 100.0f);
    fan.TouchEventRelease(103.0f, 104.0f);
    expect(!fan.IsFanOn(), "a small movement is classified as a tap");

    fan.TouchEventDown(50.0f, 50.0f);
    fan.TouchEventRelease(50.0f, 50.0f);
    expect(fan.IsFanOn(), "a second tap turns the fan on");

    fan.TouchEventDown(0.0f, 0.0f);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    fan.TouchEventMove(24.0f, 0.0f);
    expect(fan.GetMovedDistance() >= 24.0f, "drag distance is accumulated");
    expect(fan.GetDragBoost() > 0.0f, "drag velocity creates a positive boost");
    expect(fan.GetDragBoost() <= 20.0f, "drag boost is clamped");
    fan.TouchEventRelease(24.0f, 0.0f);
    expect(fan.IsFanOn(), "a drag does not toggle the fan");
    expect(std::abs(fan.GetDragBoost()) < 0.0001f, "release clears the drag boost");

    if (failures == 0) {
        std::cout << "All Fan3D interaction tests passed.\n";
    }
    return failures == 0 ? 0 : 1;
}
