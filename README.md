# XPPlugins
A collection of various small plugins for X-Plane 11 that mostly improve usability.

It currently includes
* [ToggleMouseLook](/tree/master/ToggleMouseLook)<br> Adds new commands that mimic the mouse look behaviour of Prepar3D.
* [BetterMouseYoke](/tree/master/BetterMouseLook)<br> Removes the centered + sign with the squared box around it and adds a new command that lets you toggle mouse yoke control on or off.
* [CycleQuickLooks](/tree/master/CycleQuickLooks)<br> Adds two new commands for cycling through a plane's configured quick looks.
* [PluginLoader](/tree/master/PluginLoader)<br> Enables dynamic loading and unloading of X-Plane 11 plugins under Windows.

### Building

Under Windows either simply run *msbuild.exe* or just open *XPPlugins.sln* in Visual Studio and build the solution.

For MacOSX you can just do a *make all*.