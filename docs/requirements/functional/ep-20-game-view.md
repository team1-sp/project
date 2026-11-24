---
epic: EP-20
group: srs.fr.20
part: editor
---

# EP-20 Game View

**Epic:** As a game developer, I want to see the running game at a chosen resolution with performance stats, so that I can judge how the game looks and performs.

This group covers the Game view (C-04).

## US-20.1 Game view controls

**Story:** As a game developer, I want to control the Game view's size, sound and stats, so that testing matches the real game window.

*Screens: C-04*

- **srs.fr.20.1** During Play mode, the Game view shall show the rendered game.
  - Use cases: UC-06; Initial Plan: ip.fr.4.1
- **srs.fr.20.2** When the user picks Free Aspect, 1280×720 or 1920×1080 from Resolution ▾, the Game view shall render at that size, where Free Aspect is the panel's current size in physical pixels, and letterbox it to fit the panel.
  - Use cases: UC-06; Initial Plan: ip.fr.4.1
- **srs.fr.20.3** When the user picks Custom… from Resolution ▾, the Editor shall ask for a width and height in pixels, accept whole numbers from 320 to 7 680 and from 240 to 4 320 as srs.fr.22.58 does for windows, show the reason under a refused field, and render the Game view at that size.
  - Use cases: UC-06; Initial Plan: ip.fr.4.1
- **srs.fr.20.4** When the user moves the Scale slider from 1× to 4×, the Game view shall magnify the letterboxed image of srs.fr.20.2 about the panel's centre by that factor, clipping the parts that no longer fit the panel; 1× shall show the whole image.
  - Use cases: UC-06; Initial Plan: ip.fr.4.1
- **srs.fr.20.5** When Maximize on Play is on, the Game view shall fill the main window while Play mode is on.
  - Use cases: UC-06; Initial Plan: ip.fr.4.1
- **srs.fr.20.6** When the user turns Mute on, the Editor shall set the game's audio volume to zero in the Game view without stopping playback; turning Mute off shall restore the volume.
  - Use cases: UC-06; Initial Plan: ip.fr.4.1
- **srs.fr.20.7** When the user turns Stats on, the Game view shall show an overlay with FPS (render frames per second), render frame time, physics step time, entity count and draw calls, and shall show the FPS in red when it is below the target rate: the Target FPS when VSync is off and Target FPS is above 0, or the refresh rate of the monitor showing the Game view when VSync is on and provided by the driver; with Target FPS 0, or when VSync is requested but not provided (srs.fr.4.18), there is no target rate and the FPS shall not be shown in red.
  - Use cases: UC-06; Initial Plan: ip.fr.8.3, ip.nfr.1.1
- **srs.fr.20.8** When the user clicks inside the Game view during Play mode, the Game view shall capture keyboard input for the game until the user presses Esc.
  - Use cases: UC-06; Initial Plan: ip.fr.4.1
