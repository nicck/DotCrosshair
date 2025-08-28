# DotCrosshair
Adds in a dot crosshair overlay.

## Usage
> [!NOTE]
> Check it out in action: https://youtu.be/iM_xArwz_fw

1. Download the latest release from [GitHub Releases](https://github.com/Aetopia/DotCrosshair/releases/latest).
2. Extract the `.zip` file.
3. Run `DotCrosshair.exe`.
    This will add a dot crosshair to your system tray.
    The dot overlay will appear in the center of the active window only while you hold down any mouse button (in dynamic mode). Supports left, right, middle, Mouse4, and Mouse5 buttons.

To adjust the size and color of the crosshair, do the following:
1. Create a new file called `DotCrosshair.ini` where `DotCrosshair.exe` and `DotCrosshair.dll` are located.
2. Add in the following content to the file:
    ```ini
    [Settings]
    Size = 3
    Activate = dynamic
    Color = 0,255,255
    ```
    - **Size**: The default size is 3 and minimum is 2.
    - **Activate**: Set to `"always"` for always visible dot (default), or `"dynamic"` for any mouse button activation (LMB, RMB, MMB, Mouse4, Mouse5).
    - **Color**: The default color is White (255,255,255). Use RGB format (R,G,B) where each value ranges from 0-255.
    
    **Color Examples:**
    - Red: `255,0,0`
    - Green: `0,255,0`
    - Blue: `0,0,255`
    - Yellow: `255,255,0`
    - Magenta: `255,0,255`
    - White: `255,255,255`
    - Black: `0,0,0`

## Building
1. Install [zig](https://ziglang.org/learn/getting-started/) (and or [UPX](https://upx.github.io/) for optional compression).
2. Run `Build.bat`.
