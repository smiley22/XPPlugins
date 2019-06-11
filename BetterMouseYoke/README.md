# BetterMouseYoke
A plugin that does away with X-Plane's idiotic centered little box for mouse steering and replaces it with a more sane system for those who, for whatever reason, want to or have to use the mouse for flying.

The plugin removes the centered + sign with the squared box around it and instead adds a new command to X-Plane 11 that lets you toggle mouse yoke control on or off. When toggled on, the mouse cursor changes to a <img src="yoke-mode.png?raw=true" style="position:relative; top:5px;"> symbol and you can

* move the mouse right to go right,
* move the mouse left to go left,
* move the mouse toward you to go up,
* and move the mouse away from you to go down.

The absolute mouse position on the screen represents the deviation from center of the yoke, i.e.

* move the mouse to the right edge of the screen to apply full-right yoke,
* move the mouse to the left edge of the screen to apply full-left yoke,
* move the mouse to the bottom edge of the screen to apply full-up yoke,
* and move the mouse to the top edge of the screen to apply full-down yoke.

Additionally, a magenta text indicator is displayed in the upper-left corner of the screen when mouse yoke control is active.

<p align="center">
  <a href="image-1.png?raw=true" target="_blank">
    <img title="Mouse Yoke Control" src="image-1.png?raw=true" width="860" height="503"/>
  </a>
</p>

While in mouse yoke mode, press and hold the left mouse button to switch to rudder mode. The cursor will change to a <img src="rudder-mode.png?raw=true"> symbol and the magenta text indicator will read *Mouse Rudder Control*. Two green little bars will appear to the left and to the right of the cursor that indicate the mouse movement range for rudder deflection.

<p align="center">
  <a href="image-2.png?raw=true" target="_blank">
    <img title="Mouse Rudder Control" src="image-2.png?raw=true" width="860" height="503"/>
  </a>
</p>

Release the left mouse button to transition back into mouse yoke mode. The rudder will then gradually return to the neutral position. 

### Download
You can get the latest version [here](https://github.com/smiley22/XPPlugins/releases/tag/BetterMouseYoke).

### Installation
Simply extract the .zip archive into the *plugins* directory of your *X Plane 11* installation, e.g. *X Plane 11\Resources\plugins*.

### Configuration
Once installed, you will find a new command for toggling mouse yoke control on or off in the Keyboard settings.

<p align="center">
  <a href="image-3.png?raw=true" target="_blank">
    <img title="Keyboard command" src="image-3.png?raw=true" width="860" height="503"/>
  </a>
</p>

##### Config File
On startup, the plugin looks for a *settings.ini* file in its' directory, i.e. *"X Plane 11\Resources\plugins\BetterMouseYoke\settings.ini"*. This file allows you to change some of the settings of the plugin. If the file does not exist, the plugin uses default values.

Here's an example *settings.ini* file with all the options the plugin allows you to configure. Lines starting with a **#** are comments.

```
[settings]
# When entering mouse yoke mode, set the mouse cursor to align with
# the current yoke deflection to avoid rapid movements.
# Set to 0 to disable.
set_pos = 1
# Change the mouse cursor symbol when in yoke or rudder mode. Set
# this to 0 to disable.
change_cursor = 1
# Defines the mouse movement range for rudder deflection in rudder mode.
# In other words, the greater the value, the greater the distance between
# the two green vertical bars on the screen.
rudder_deflection_distance = 100
# The speed (in units per second) with which the rudder returns to the
# neutral position after exiting rudder mode.
# The default value of 2.0 means that it takes the rudder half a second
# to return to neutral from full left or right deflection.
rudder_return_speed = 2.0
```

