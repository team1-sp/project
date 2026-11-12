// PB-008 spike: Box2D simulates a ground and a falling box, raylib draws them.
// Usage: box2d_raylib_spike [--frames N]   (N = 0, the default, runs until the window is closed)
#include <cstdlib>
#include <string_view>

#include <raylib.h>

#include "falling_box_scene.hpp"
#include "world_to_screen.hpp"

namespace {

constexpr int screenWidthPixels = 800; // 25 m wide at 32 px per meter
constexpr int screenHeightPixels = 450;

void drawBody(const sandbox::BodyPose& pose, sandbox::BoxSize size, Color color) {
    const sandbox::ScreenPoint center =
        sandbox::worldToScreen(pose.xMeters, pose.yMeters, static_cast<float>(GetScreenHeight()));
    const float widthPixels = sandbox::metersToPixels(size.widthMeters);
    const float heightPixels = sandbox::metersToPixels(size.heightMeters);
    // DrawRectanglePro puts `origin` at (rec.x, rec.y) and rotates around it. With the origin at
    // the rectangle's centre, rec.x/rec.y is the body position, which is the body's centre here.
    DrawRectanglePro(Rectangle{center.xPixels, center.yPixels, widthPixels, heightPixels},
                     Vector2{widthPixels / 2.0f, heightPixels / 2.0f},
                     sandbox::worldAngleToScreenDegrees(pose.angleRadians), color);
}

} // namespace

int main(int argc, char** argv) {
    int maxFrames = 0;
    for (int i = 1; i < argc; ++i) {
        if (std::string_view(argv[i]) == "--frames" && i + 1 < argc) {
            maxFrames = std::atoi(argv[++i]);
        }
    }

    InitWindow(screenWidthPixels, screenHeightPixels, "PB-008 spike: Box2D + raylib");
    // raylib is built with SUPPORT_CUSTOM_FRAME_CONTROL (DEC-051): EndDrawing() does not
    // present, cap the frame rate or poll input, and GetFrameTime() stays 0, so this loop does
    // each of them itself.
    constexpr double targetFrameSeconds = 1.0 / 60.0;
    double previousFrameStart = GetTime();

    sandbox::FallingBoxScene scene;
    float accumulatorSeconds = 0.0f;
    constexpr float maxCatchUpSeconds = 0.25f; // after a long stall, drop time instead of freezing

    int frame = 0;
    while (!WindowShouldClose() && (maxFrames == 0 || frame < maxFrames)) {
        PollInputEvents();
        const double frameStart = GetTime();
        // Physics runs at a fixed 1/60 s whatever the frame rate is (R-5): the frame time only
        // decides how many fixed steps to run.
        accumulatorSeconds += static_cast<float>(frameStart - previousFrameStart);
        previousFrameStart = frameStart;
        if (accumulatorSeconds > maxCatchUpSeconds) {
            accumulatorSeconds = maxCatchUpSeconds;
        }
        while (accumulatorSeconds >= sandbox::FallingBoxScene::timeStepSeconds) {
            scene.step();
            accumulatorSeconds -= sandbox::FallingBoxScene::timeStepSeconds;
        }

        const sandbox::BodyPose box = scene.boxPose();
        BeginDrawing();
        ClearBackground(RAYWHITE);
        drawBody(scene.groundPose(), sandbox::FallingBoxScene::groundSize, DARKGRAY);
        drawBody(box, sandbox::FallingBoxScene::boxSize, MAROON);
        DrawText(TextFormat("box y = %.2f m   angle = %.2f rad   %s",
                            static_cast<double>(box.yMeters), static_cast<double>(box.angleRadians),
                            scene.boxIsAwake() ? "awake" : "asleep"),
                 20, 20, 20, DARKGRAY);
        DrawText(TextFormat("contacts begun: %d", static_cast<int>(scene.contactBegins().size())),
                 20, 45, 20, DARKGRAY);
        EndDrawing();
        SwapScreenBuffer();
        const double drawSeconds = GetTime() - frameStart;
        if (drawSeconds < targetFrameSeconds) {
            WaitTime(targetFrameSeconds - drawSeconds);
        }
        ++frame;
    }

    CloseWindow();
    return 0;
}
