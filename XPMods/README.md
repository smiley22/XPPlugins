Synopsis
========

This script allows you to easily install and/or uninstall mods that replace or modify existing files.

![](image.png?raw=true)

Description
===========

The script uses file-system links to install mods so that no files need to be copied around and no extra disk-space is taken up. It will also make a backup of every file that is modified so that you can easily uninstall a mod again without messing up your installation. The script will look for mods in its executing directory. A mod in this context is simply a directory that contains the files that make up the mod.

Using `XPMods`
--------------------

Invoking `XPMods` without any command-line arguments will displays an interactive prompt from which you can then enable and/or disable mods.

    PS> ./XPMods.ps1

You can also pass mods to enable or disable directly through the command-line without having to go through the interactive menu by using the `-Enable` and `-Disable` options:

    PS> ./XPMods.ps1 -Disable "Autumn" -Enable "Winter" -Enable "Snow Effects"

In order to enable or disable all available mods, you can use the `-EnableAll` or `-DisableAll` options. Disabling all mods will restore all files back to their original, unmodified state:

    PS> ./XPMods.ps1 -DisableAll

For a detailed description of all available options, please refer to the next section.

Options
=======

`-EnableAll`  
Enables all mods. Mods that are already enabled, will remain enabled.

`-DisableAll`  
Disables all mods and restores original files.

`-NoWarn`  
Don't warn when a mod replaces another mod's file-system link.

`-Verbose`  
Prints verbose debug messages to console.

`-Enable` *mod*  
Specifies a mod to enable. You can use this option multiple times on the command-line to enable several mods at once.

`-Disable` *mod*  
Specifies a mod to disable. You can use this option multiple times on the command-line to disable several mods at once.

`-Version`  
Prints version information.

Authors
=======

© 2019 Torben Könke (torben dot koenke at gmail dot com).

License
=======

This program is released under the MIT license.
