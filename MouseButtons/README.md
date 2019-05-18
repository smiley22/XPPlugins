# MouseButtons
A plugin that frees up the underused mouse in X-Plane 11 and let's you bind arbitrary commands to any of your mouse buttons and/or mouse wheel.
 
The plugin uses the same kind of *.prf* preference files as X-Plane for assigning commands to mouse buttons. Whenever you load up an aircraft, the plugin will look for a preference file with the same name as the aircraft inside the plugin's directory. In other words, when you jump into the *Cessna 172SP* the plugin will look for a preference file *"X Plane 11/Resources/plugins/MouseButtons/Cessna 172SP.prf"*. If it can't find an aircraft-specific .prf file, it will look for a generic *mouse.prf* in the same directory.

The format of an entry in the preference file is as follows:

<table>
 <tr>
  <td>button-to-bind</td>
  <td>modifier-keys</td>
  <td>command-to-exec</td>
 </tr>
</table>

Available button identifiers are:

* Mouse-Left
* Mouse-Right
* Mouse-Middle            *(Scroll wheel click)*
* Mouse-Forward           *(Thumb forward button)*
* Mouse-Backward          *(Thumb backward button)*
* Mouse-Wheel-Forward     *(Scroll wheel forward)*
* Mouse-Wheel-Backward    *(Scroll wheel backward)*
* Mouse-Wheel-Left        *(Tilt scroll wheel left)*
* Mouse-Wheel-Right       *(Tilt scroll wheel right)*

Available modifier keys are:

* CTRL
* SHIFT
* ALT
* LMB      *(Left mouse button)*
* RMB      *(Right mouse button)*
* MMB      *(Scroll wheel button)*
* FMB      *(Thumb forward button)*
* BMB      *(Thumb backward button)*
* or &lt;NONE&gt; for no modifier key

Multiple modifier keys can be combined with +, so an entry like

    Mouse-Middle   CTRL+FMB    sim/view/default_view
    
would only trigger if you held down both CTRL and the thumb forward button when clicking the mouse wheel button.

### Example preference file

The following is an example preference file.

    I
    1005 Version
    Mouse-Middle            <NONE>      BetterMouseYoke/ToggleYokeControl
    Mouse-Forward           <NONE>      sim/engines/throttle_down
    Mouse-Backward          <NONE>      sim/engines/throttle_up
    Mouse-Wheel-Left        <NONE>      sim/autopilot/servos_toggle
    Mouse-Wheel-Right       <NONE>      sim/view/quick_look_1
    Mouse-Wheel-Forward     LMB         CycleQuickLooks/Forward
    Mouse-Wheel-Backward    LMB         CycleQuickLooks/Backward

### Download
You can get the latest version [here](https://github.com/smiley22/XPPlugins/releases/tag/MouseButtons).

### Installation
Simply extract the .zip archive into the *plugins* directory of your *X Plane 11* installation, e.g. *X Plane 11\Resources\plugins*.
