# PluginLoader
Enables dynamic loading and unloading of X-Plane 11 plugins.

This plugin enables dynamic loading und unloading of plugins dll so that one does not need to constantly restart X-Plane 11 during development and testing of plugins under Windows.

The loader looks for any dll files in its directory and attempts to load them as XP11 plugins **without locking them**. Once a plugin dll has been loaded, the loader then forwards all calls to X-Plane 11's *PLUGIN_API* function callbacks to the loaded plugin.

The plugin also adds the command _Plugin/Reload_ to XP11 that unloads all loaded plugins from memory and then reloads them from their respective dll file from disk.


![alt text](image.jpg?raw=true)