---
epic: EP-04
group: srs.fr.4
part: engine
---

# EP-04 Rendering

**Epic:** As a game developer, I want to see every visible entity drawn in the game window, so that I can watch and judge my game.

These requirements refine ip.fr.4.1 to ip.fr.4.4. Rendering uses raylib 6.2 behind the rendering abstraction.

## US-4.1 Sprites

**Story:** As a game developer, I want entities to be drawn from their sprite and position data, so that what I see matches the ECS state.

- **srs.fr.4.1** In normal mode, the Engine shall draw every enabled entity that has a Position component and an enabled Sprite component at its world position, rotation and scale, composed through its parents as stated in srs.fr.3.37.
  - Use cases: UC-06; Initial Plan: ip.fr.4.1
- **srs.fr.4.2** The Engine shall draw sprites in ascending order of their Sprite "order in layer" value and, for equal values, in scene order and then Hierarchy order, where scene order is the active scene, then the persistent entities, then the other loaded scenes in the order they were loaded.
  - Use cases: UC-06; Initial Plan: ip.fr.4.1
- **srs.fr.4.3** The Engine shall apply the tint colour and the horizontal and vertical flip flags of the Sprite component when drawing it.
  - Use cases: UC-06; Initial Plan: ip.fr.4.1
- **srs.fr.4.4** For an entity with an enabled Unlockable component, the Engine shall draw, while is unlocked is false, its locked sprite and, while it is true, its unlocked sprite (srs.fr.22.44); when that field is empty, it shall draw the Sprite component's state sprite for the state Locked or Unlocked (srs.fr.22.36), and when that is also missing, the Sprite component's sprite asset. For any other entity, the Engine shall draw the sprite asset and ignore the state sprites.
  - Use cases: UC-06; Initial Plan: ip.fr.4.2
- **srs.fr.4.5** The rendering subsystem shall only read ECS data and shall not change any simulation state.
  - Use cases: UC-06; Initial Plan: ip.fr.4.3

## US-4.2 Camera and output

**Story:** As a game developer, I want to control what part of the scene is shown and how it is drawn, so that the game looks the way I configured it.

- **srs.fr.4.6** The Engine shall draw each render frame through the view transform of the active camera (srs.fr.4.25), chosen by srs.fr.4.12.
  - Use cases: UC-04, UC-06; Initial Plan: ip.fr.4.4
- **srs.fr.4.7** When the active camera has a follow target, the Engine shall, in every render frame, set the view transform's position to the target entity's world position after the last completed tick.
  - Use cases: UC-04, UC-06; Initial Plan: ip.fr.4.4
- **srs.fr.4.8** When the active camera has bounds, the Engine shall then clamp the view transform so that the visible area stays inside those bounds, and shall centre the view on the bounds along any axis in which the visible area is larger than the bounds.
  - Use cases: UC-04, UC-06; Initial Plan: ip.fr.4.4
- **srs.fr.4.9** The Engine shall clear each render frame with the clear colour from the project's Rendering settings.
  - Use cases: UC-04, UC-06; Initial Plan: ip.fr.4.1
- **srs.fr.4.10** In normal mode, the Engine shall limit only the render frame rate, never the length of a simulation tick: when VSync is on, it shall request presentation synchronized with the monitor refresh rate and ignore Target FPS; when VSync is off, it shall limit render frames to the Target FPS value, where 0 means no limit.
  - Use cases: UC-04, UC-06; Initial Plan: ip.fr.4.1
- **srs.fr.4.11** The Engine shall sample each texture with the asset's own filter, or with the project's Texture filter setting when the asset's filter is Project default (srs.fr.23.37, srs.fr.27.22).
  - Use cases: UC-04, UC-06; Initial Plan: ip.fr.4.1

## US-4.3 Camera selection and render frame rate

**Story:** As a game developer, I want a clear rule for which camera is used and how fast render frames are drawn, so that the game view never goes blank or behaves differently on another monitor.

- **srs.fr.4.12** The active camera shall be the first enabled Camera component on an enabled entity, searching first the active scene in Hierarchy order, then the persistent entities, then the other loaded scenes in the order they were loaded, each in Hierarchy order.
  - Use cases: UC-06; Initial Plan: ip.fr.4.1
- **srs.fr.4.13** When more than one camera is eligible, the Engine shall use the active camera of srs.fr.4.12 and log one Info message per run that names the cameras that were not used.
  - Use cases: UC-06; Initial Plan: ip.fr.4.1
- **srs.fr.4.14** When the active camera is disabled or destroyed, the Engine shall choose a new active camera by srs.fr.4.12 in the next render frame.
  - Use cases: UC-06; Initial Plan: ip.fr.4.1
- **srs.fr.4.15** When no camera is eligible, the Engine shall draw the scene with a default view centred on world position (0, 0) at zoom 1 and log the warning "No active camera" once per run.
  - Use cases: UC-06; Initial Plan: ip.fr.4.1
- **srs.fr.4.16** When the active camera's follow target is lost, because the target reference is set but the entity is disabled or not found, or because the Engine cleared the reference when the target was destroyed or unloaded (srs.fr.2.44, srs.fr.4.28), the view shall stay at the last followed position retained for that camera (srs.fr.4.27), or at the camera's authored world position when it has not followed in this session, and the Engine shall log one warning per run that names the camera; follow shall resume when a target becomes available again. A camera whose follow target was never set, or was set to empty by a system or the user, shall use its authored world position and log no warning.
  - Use cases: UC-06; Initial Plan: ip.fr.4.1
- **srs.fr.4.17** Camera follow and bounds shall not be part of the simulation: they shall change only the view transform (srs.fr.4.25), run per render frame and not run in headless mode.
  - Use cases: UC-06; Initial Plan: ip.fr.4.1
- **srs.fr.4.18** When the graphics driver does not provide VSync, or the computer cannot reach the requested render frame rate, the Engine shall run at the rate it achieves, keep simulation ticks at 1/60 s by srs.fr.1.3 and show the achieved rate in the Stats overlay (srs.fr.20.7); no render frame rate shall be guaranteed.
  - Use cases: UC-06; Initial Plan: ip.fr.4.1

## US-4.4 Audio

**Story:** As a game developer, I want to minimal sound playback for my game, so that the demo can play effects and music.

- **srs.fr.4.19** The Engine shall provide an Audio Source component with the fields audio clip (a wav asset), volume (0 to 1), loop and play on start.
  - Use cases: UC-06; Initial Plan: —
- **srs.fr.4.20** In normal mode, when a run starts, the Engine shall play the clip of every enabled Audio Source whose play on start is on, and shall repeat it when loop is on.
  - Use cases: UC-06; Initial Plan: —
- **srs.fr.4.21** The Engine shall let systems start and stop an Audio Source's clip; audio output shall never change simulation state.
  - Use cases: UC-06; Initial Plan: —
- **srs.fr.4.22** When an Audio Source or its entity is disabled or destroyed, the Engine shall stop its clip.
  - Use cases: UC-06; Initial Plan: —
- **srs.fr.4.23** In headless mode, the Engine shall not open an audio device and shall ignore requests to start or stop clips.
  - Use cases: UC-06; Initial Plan: —
- **srs.fr.4.24** The Engine shall play audio through the audio functions of raylib behind the platform abstraction, so that no other engine code calls the audio library directly.
  - Use cases: UC-06; Initial Plan: —

## US-4.5 View transform

**Story:** As a game developer, I want to camera following to change only what I see, so that rendering never changes the simulation.

- **srs.fr.4.25** In every render frame, the Engine shall compute the active camera's view transform from its zoom and a start position, which shall be the camera entity's world position or, while its configured follow target is unavailable, the position of srs.fr.4.16, then apply follow (srs.fr.4.7) and then bounds (srs.fr.4.8); the view transform shall be kept outside the ECS.
  - Use cases: UC-06; Initial Plan: ip.fr.4.1
- **srs.fr.4.26** In Edit mode, the Game view shall show the camera's authored view with bounds applied and without follow.
  - Use cases: UC-06; Initial Plan: ip.fr.4.1

## US-4.6 Retained follow position

**Story:** As a game developer, I want the camera to stay where it was when its target goes away, so that the view does not jump back to the authored start.

- **srs.fr.4.27** The Engine shall retain, outside the ECS, the last followed view position of each camera entity for the current Play mode session or run, set in every render frame in which follow is applied, and clear it when the session or run starts.
  - Use cases: UC-06; Initial Plan: ip.fr.4.1

## US-4.7 Lost follow targets

**Story:** As a game developer, I want the camera to tell a lost target from no target, so that the view stays put when its target is destroyed.

- **srs.fr.4.28** The Engine shall record, outside the ECS and with the retained position of srs.fr.4.27, each camera whose follow target reference it cleared because the target was destroyed or unloaded, and treat that camera as having a lost target until a system or the user sets the field again.
  - Use cases: UC-06; Initial Plan: ip.fr.4.1
