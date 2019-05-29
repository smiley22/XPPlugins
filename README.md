# XPPlugins
A collection of various small plugins for X-Plane 11 that mostly improve usability.

It currently includes
* [ToggleMouseLook](ToggleMouseLook)<br> Adds new commands that mimic the mouse look behaviour of Prepar3D.
* [BetterMouseYoke](BetterMouseYoke)<br> Removes the centered + sign with the squared box around it and adds a new command that lets you toggle mouse yoke control on or off.
* [MouseButtons](MouseButtons)<br> A plugin that frees up the underused mouse in X-Plane 11 and let's you bind arbitrary commands to any of your mouse buttons and/or mouse wheel.
* [CycleQuickLooks](CycleQuickLooks)<br> Adds two new commands for cycling through a plane's configured quick looks.
* [PluginLoader](PluginLoader)<br> Enables dynamic loading and unloading of X-Plane 11 plugins under Windows.
* [A320UE](A320UE)<br> Adds a couple of new commands and other tweaks to the *FF A320 Ultimate* to make it even more enjoyable to fly.
* [XPMods](XPMods)<br> A PowerShell script for safely installing and/or uninstalling mods that replace or modify existing XP files.

### Building

Under Windows either simply run *msbuild.exe* or just open *XPPlugins.sln* in Visual Studio and build the solution.

For MacOSX you can just do a *make all*.