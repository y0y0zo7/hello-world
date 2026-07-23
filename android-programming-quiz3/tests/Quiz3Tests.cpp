/**
 * @file Quiz3Tests.cpp
 * @brief Verifies Quiz 3 control state, clamps, selection, and bonus behavior.
 * @author Zoey <t.zoey@digipen.edu>
 */

#include "Fan.h"
#include "Scene3D.h"
#include "SceneHUD.h"

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

void press(SceneHUD& hud, int button)
{
    const glm::vec2 center = hud.ButtonCenter(button);
    expect(hud.TouchDown(center.x, center.y), "button center is hit-testable");
    hud.TouchRelease();
}
} // namespace

int main()
{
    Fan fan;
    expect(std::abs(fan.GetSpeed() - 8.0f) < 0.001f, "fan starts at speed 8");
    fan.SetSpeed(30.0f);
    expect(std::abs(fan.GetSpeed() - 20.0f) < 0.001f, "fan speed clamps to 20");
    fan.SetSpeed(-4.0f);
    expect(std::abs(fan.GetSpeed()) < 0.001f, "fan speed clamps to zero");
    expect(fan.GetSelectedPart() == Fan::NoPart, "fan starts with no picked part");
    fan.SelectPart(Fan::Blade2);
    expect(fan.GetSelectedPart() == Fan::Blade2, "fan stores an exact blade selection");
    fan.SelectPart(99);
    expect(fan.GetSelectedPart() == Fan::NoPart, "invalid selection clears highlighting");

    SceneHUD hud;
    hud.Resize(800, 700);
    expect(hud.CurrentSpeed() == 8, "HUD starts with eight active speed blocks");
    expect(hud.ConsumeZoomSteps() == 0, "HUD starts with no pending camera action");
    expect(!hud.TouchDown(400.0f, 350.0f), "empty scene input misses the HUD");

    press(hud, SceneHUD::SpeedUp);
    expect(hud.CurrentSpeed() == 9, "+SPD increments speed");
    press(hud, SceneHUD::SpeedDown);
    expect(hud.CurrentSpeed() == 8, "-SPD decrements speed");

    for (int i = 0; i < 30; ++i) press(hud, SceneHUD::SpeedUp);
    expect(hud.CurrentSpeed() == 20, "+SPD never exceeds 20");
    for (int i = 0; i < 30; ++i) press(hud, SceneHUD::SpeedDown);
    expect(hud.CurrentSpeed() == 0, "-SPD never drops below zero");

    press(hud, SceneHUD::ZoomIn);
    expect(hud.ConsumeZoomSteps() == -1, "+ZOOM emits a camera-nearer action");
    expect(hud.ConsumeZoomSteps() == 0, "zoom actions are consumed exactly once");
    press(hud, SceneHUD::ZoomOut);
    expect(hud.ConsumeZoomSteps() == 1, "-ZOOM emits a camera-farther action");

    Scene3D scene;
    scene.SetCameraDistance(-100.0f);
    expect(std::abs(scene.CameraDistance() - 4.0f) < 0.001f,
           "Scene3D owns and clamps the near camera limit");
    scene.SetCameraDistance(100.0f);
    expect(std::abs(scene.CameraDistance() - 18.0f) < 0.001f,
           "Scene3D owns and clamps the far camera limit");

    expect(hud.PickedPart() == -1, "HUD starts with no pick feedback");
    hud.SetPickedPart(Fan::Blade3);
    expect(hud.PickedPart() == Fan::Blade3, "HUD reports the selected fan part");
    hud.SetPickedPart(99);
    expect(hud.PickedPart() == -1, "invalid pick feedback clears the status strip");

    SceneHUD heldHud;
    heldHud.Resize(800, 700);
    const glm::vec2 speedUpCenter = heldHud.ButtonCenter(SceneHUD::SpeedUp);
    expect(heldHud.TouchDown(speedUpCenter.x, speedUpCenter.y), "press-and-hold starts on +SPD");
    const int afterInitialPress = heldHud.CurrentSpeed();
    std::this_thread::sleep_for(std::chrono::milliseconds(370));
    expect(heldHud.UpdateHeldAction(), "held button repeats after its initial delay");
    expect(heldHud.CurrentSpeed() == afterInitialPress + 1,
           "press-and-hold performs one additional speed step");
    expect(heldHud.UpdateHeldAction(), "held button remains level-triggered every frame");
    expect(heldHud.CurrentSpeed() == afterInitialPress + 2,
           "per-frame hold performs another speed step without a timer gap");
    heldHud.TouchRelease();
    expect(!heldHud.UpdateHeldAction(), "release cancels press-and-hold");

    if (failures == 0) {
        std::cout << "All Programming Quiz 3 state and HUD tests passed.\n";
    }
    return failures == 0 ? 0 : 1;
}
