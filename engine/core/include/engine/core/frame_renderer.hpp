#pragma once

namespace engine {

// Draws one render frame from the state the last completed tick left (srs.fr.1.1, 1.15). The
// game loop calls it once per render frame, between IWindow::beginFrame() and presentFrame(), in
// normal mode only; a headless run never calls it (srs.fr.7.1). An implementation reads
// simulation state and must not change it.
class IFrameRenderer {
public:
    virtual ~IFrameRenderer() = default;

    virtual void renderFrame() = 0;
};

} // namespace engine
