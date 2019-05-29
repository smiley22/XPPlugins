<#

.SYNOPSIS
This script allows you to easily install and/or uninstall mods that replace
or modify existing files.

.DESCRIPTION
The script uses file-system links to install mods so that no files need to be
copied around and no extra disk-space is taken up. It will also make a backup
of every file that is modified so that you can easily uninstall a mod again
without messing up your installation. The script will look for mods in its
executing directory. A mod in this context is simply a directory that contains
the files that make up the mod.

.PARAMETER EnableAll
Enables all mods.

.PARAMETER DisableAll
Disables all mods and restores original files.

.PARAMETER NoWarn
Don't warn when a mod replaces another mod's file-system link.

.PARAMETER Verbose
Print verbose messages to console.

.PARAMETER Version
Print version information.

.PARAMETER Enable
Specifies a mod to enable.

.PARAMETER Disable
Specifies a mod to disable.

.EXAMPLE
./XPMods.ps1
Displays an interactive prompt for enabling/disabling mods.

.EXAMPLE
./XPMods.ps1 -Enable "Winter Package"
Enables the mod "Winter Package". Expects a directory "Winter Package" with
the mod's files in the same directory as the script.

.EXAMPLE
./XPMods.ps1 -DisableAll
Disables all mods that are currently enabled. All original XP files will be
restored.

.EXAMPLE
./XPMods.ps1 -EnableAll -Verbose
Enables all mods and prints verbose messages to the console during
installation.

.EXAMPLE
./XPMods.ps1 -Disable "Autumn" -Enable "Winter" -Enable "Snow Effects"
Multiple mods can be enabled and/or disabled with a single script invocation.

.NOTES
When invoked without any arguments, the script will show an interactive prompt
that allows you to enable/disable individual mods and then apply the changes.

.LINK
https://github.com/smiley22/XPPlugins

#>

param(
  [switch]   $EnableAll,
  [switch]   $DisableAll,
  [switch]   $NoWarn,
  [switch]   $Verbose,
  [switch]   $Version,
  [string[]] $Enable,
  [string[]] $Disable
)

$_Version = 1.0
$XPRoot   = [IO.Path]::GetFullPath(
  [IO.Path]::Combine($PSScriptRoot, "..")
)

function main() {
  if ($Script:PSBoundParameters.Count -eq 0) {
    # If no arguments passed, show selection menu
    showMenu
  } else {
    if ($Version) {
      Write-Host ("XPMods.ps1 v{0:f1}" -f $_Version) -ForegroundColor Green
    }
    if ($EnableAll) {
      $Enable = collectAll
    }
    if ($DisableAll) {
      $Disable = collectAll
    }
    foreach ($m in $Disable) {
      disableMod $m
    }
    foreach ($m in $Enable) {
      enableMod $m
    }
  }
}

function collectAll() {
  $dirs = [IO.Directory]::EnumerateDirectories($PSScriptRoot)
  $ret = @()
  foreach ($d in $dirs) {
    $ret += [IO.Path]::GetFileName($d)
  }
  return $ret
}

function enableMod($mod) {
  Write-Host "Enabling mod " -NoNewLine
  Write-Host $mod -NoNewLine -ForegroundColor Magenta
  Write-Host " ..." -NoNewLine
  # Get all files in mod's directory.
  $path = [IO.Path]::Combine($PSScriptRoot, $mod)
  $files = [IO.Directory]::EnumerateFiles($path, "*.*", "AllDirectories")
  foreach ($f in $files) {
    $t = $f.Replace($path, "").TrimStart('\')
    $m = [IO.Path]::Combine($XPRoot, $t)
    if ([IO.File]::Exists($m)) {
      if (isLink $m) {
        $target = getLinkTarget $m
        if ($target -ne $f) {
          if (!$NoWarn) {
            warn $m $target $f
          }
          [IO.File]::Delete($m)
          createLink $m $f
        }
      } else {
        # Create backup of original file
        if($Verbose) {
          Write-Host "Backing up original file " -NoNewLine
          Write-Host $m -ForegroundColor Cyan
        }
        [IO.File]::Move($m, "$m.orig")
        createLink $m $f
      }
    } else {
      createLink $m $f
    }
  }
  $path = [IO.Path]::Combine($XPRoot, "Resources", "mods")
  [IO.Directory]::CreateDirectory($path) >$null
  [IO.File]::Create(
    [IO.Path]::Combine($path, $mod)
  ).Dispose() >$null
  Write-Host "done" -ForegroundColor Green
}

function disableMod($mod) {
  Write-Host "Disabling mod " -NoNewLine
  Write-Host $mod -NoNewLine -ForegroundColor Yellow
  Write-Host " ..." -NoNewLine
  # Get all files in mod's directory.
  $path = [IO.Path]::Combine($PSScriptRoot, $mod)
  $files = [IO.Directory]::EnumerateFiles($path, "*.*", "AllDirectories")
  foreach ($f in $files) {
    $t = $f.Replace($path, "").TrimStart('\')
    $m = [IO.Path]::Combine($XPRoot, $t)
    if (!([IO.File]::Exists($m))) {
      continue
    }
    if (isLink $m) {
      $target = getLinkTarget $m
      if ($target -eq $f) {
       # m is a link pointing to us
       [IO.File]::Delete($m)
       # Restore original file if it exists
       $orig = "$m.orig"
       if (([IO.File]::Exists($orig))) {
         [IO.File]::Move($orig, $m)
         if($Verbose) {
           Write-Host "Restoring original file " -NoNewLine
           Write-Host $m -ForegroundColor Cyan
         }
       }
      }
    }
  }
  $path = [IO.Path]::Combine($XPRoot, "Resources", "mods", $mod)
  if([IO.File]::Exists($path)) {
    [IO.File]::Delete($path)
  }
  Write-Host "done" -ForegroundColor Green
}


function createLink($name, $target) {
  if($Verbose) {
    Write-Host "Linking " -NoNewLine
    Write-Host $name -NoNewLine -ForegroundColor Cyan
    Write-Host " to " -NoNewLine
    Write-Host $target -ForegroundColor Magenta
  }
  $dir = [IO.Path]::GetDirectoryName($name)
  [IO.Directory]::CreateDirectory($dir) >$null
  [Kernel32]::CreateHardLink($name, $target, [IntPtr]::Zero) >$null
}

function isLink($path) {
  $count = [Kernel32]::GetFileLinkCount($path)
  return ($count -gt 1)
}

function getLinkTarget($path) {
  $siblings = [Kernel32]::GetFileSiblingHardLinks($path)
  if ($siblings.Count -gt 0) {
    return $siblings[0]
  }
  return $null
}

function warn($path, $existing, $newTarget) {
  Write-Host "[" -NoNewLine
  Write-Host "Warning" -NoNewLine -ForegroundColor Yellow
  Write-Host "] " 
  Write-Host $path -NoNewLine -ForegroundColor Cyan
  Write-Host " was a link to file " -NoNewLine
  Write-Host $existing -ForegroundColor Yellow
  Write-Host "It now points to file " -NoNewLine
  Write-Host $newTarget -ForegroundColor Magenta
}

function getMods() {
  $dirs = [IO.Directory]::EnumerateDirectories($PSScriptRoot)
  $mods = @()
  foreach($d in $dirs) {
    $mods += [IO.Path]::GetFileName($d)
  }
  return $mods
}

function writeLine($s = "", $color = "White") {
  $current = [Console]::ForegroundColor
  [Console]::ForegroundColor = $color
  [Console]::WriteLine($s)
  [Console]::ForegroundColor = $current
}

function isEnabled($mod) {
  $path = [IO.Path]::Combine($XPRoot, "Resources", "mods", $mod)
  return ([IO.File]::Exists($path))
}

function showMenu() {
  $mods = @(getMods)
  $enabled = @()
  foreach ($m in $mods) {
    $enabled += (isEnabled $m)
  }
  if ($mods.Length -eq 0) {
    writeLine "No mods found" "Red"
    writeLine ""
    writeLine "Move mods into '$PSScriptRoot' for them to show up here."
    return
  }
  [Console]::CursorVisible = $false
  [Console]::Clear()
  writeLine ("XPMods.ps1 v{0:f1}" -f $_Version) "Green"
  writeLine "Use the arrow keys to move up and down in the list. Press SPACE to enable or disable an entry."
  writeLine ""
  $offset = [Console]::CursorTop
  $cursor = 0
  $confirm = $false
  for ($i = 0; $i -lt $mods.Length; $i++) {
    # Powershell is so terrible it boggles the mind.
    $s = ($(if ($enabled[$i]) {"[*] "} else {"[ ] "}) + $mods[$i])
    $c = $(if ($i -eq $cursor) {"Yellow"} else {"White"})
    writeLine $s $c
  }

  writeLine ""
  writeLine "When done, press ENTER to apply your changes."
  $endOffset = [Console]::CursorTop + 1
  while($true) {
    $k = [Console]::ReadKey($true).Key
    $prev = $cursor
    switch($k) {
      "UpArrow" {
        if ($cursor -gt 0) {
          $cursor--
        }
      }
      "DownArrow" {
        if ($cursor -lt $mods.Length - 1) {
          $cursor++
        }
      }
      "Spacebar" {
        $enabled[$cursor] = !$enabled[$cursor]
      }
      "Enter" {
        if ($confirm) {
          applySelection $mods $enabled
          [Console]::CursorVisible = $true
          return
        } else {
          $confirm = $true
          [Console]::SetCursorPosition(0, $offset + $mods.Length + 1)
          writeLine "Confirm your selection by pressing ENTER again, or press ESC to cancel." "Yellow"
        }
      }
      "Escape" {
        if ($confirm) {
          $confirm = $false
          [Console]::SetCursorPosition(0, $offset + $mods.Length + 1)
          [Console]::Write("".PadLeft([Console]::WindowWidth), ' ')
          [Console]::SetCursorPosition(0, $offset + $mods.Length + 1)
          writeLine "When done, press ENTER to apply your changes."
        } else {
          [Console]::CursorVisible = $true
          [Console]::SetCursorPosition(0, $endOffset)
          return
        }
      }
    }
    [Console]::SetCursorPosition(0, $offset + $prev)
    writeLine ($(if ($enabled[$prev]) {"[*] "} else {"[ ] "}) + $mods[$prev])
    [Console]::SetCursorPosition(0, $offset + $cursor)
    writeLine ($(if ($enabled[$cursor]) {"[*] "} else {"[ ] "}) + $mods[$cursor]) "Yellow"
  }
}

function applySelection($mods, $enable) {
  [Console]::Clear()
  for($i = 0; $i -lt $mods.Length; $i++) {
    $current = isEnabled $mods[$i]
    if ($current -ne $enable[$i]) {
      if ($enable[$i]) {
        enableMod $mods[$i]
      } else {
        disableMod $mods[$i]
      }
    }
  }
  writeLine ""
  writeLine "All done"
}

#
# WIN32 API Boilerplate for dealing with hard-links.
#
function addKernel32Type() {
  try {
    Add-Type @"
      using System;
      using System.Collections.Generic;
      using System.IO;
      using System.Runtime.InteropServices;
      using System.Text;
      using Microsoft.Win32.SafeHandles;
      using FILETIME = System.Runtime.InteropServices.ComTypes.FILETIME;
      public static class Kernel32 {
        [StructLayout(LayoutKind.Sequential)]
        public struct BY_HANDLE_FILE_INFORMATION {
            public uint FileAttributes;
            public FILETIME CreationTime;
            public FILETIME LastAccessTime;
            public FILETIME LastWriteTime;
            public uint VolumeSerialNumber;
            public uint FileSizeHigh;
            public uint FileSizeLow;
            public uint NumberOfLinks;
            public uint FileIndexHigh;
            public uint FileIndexLow;
        }
        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern bool CreateHardLink(string lpFileName,
            string lpExistingFileName, IntPtr lpSecurityAttributes);
        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        static extern SafeFileHandle CreateFile(
            string lpFileName,
            [MarshalAs(UnmanagedType.U4)] FileAccess dwDesiredAccess,
            [MarshalAs(UnmanagedType.U4)] FileShare dwShareMode,
            IntPtr lpSecurityAttributes,
            [MarshalAs(UnmanagedType.U4)] FileMode dwCreationDisposition,
            [MarshalAs(UnmanagedType.U4)] FileAttributes dwFlagsAndAttributes,
            IntPtr hTemplateFile);
        [DllImport("kernel32.dll", SetLastError = true)]
        static extern bool GetFileInformationByHandle(SafeFileHandle handle,
            out BY_HANDLE_FILE_INFORMATION lpFileInformation);
        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        static extern IntPtr FindFirstFileNameW(
            string lpFileName, uint dwFlags, ref uint stringLength,
            StringBuilder fileName);
        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        static extern bool FindNextFileNameW(
            IntPtr hFindStream, ref uint stringLength, StringBuilder fileName);
        [DllImport("kernel32.dll", SetLastError = true)]
        static extern bool FindClose(IntPtr fFindHandle);
        [DllImport("kernel32.dll")]
        static extern bool GetVolumePathName(string lpszFileName,
            [Out] StringBuilder lpszVolumePathName, uint cchBufferLength);
        [DllImport("shlwapi.dll", CharSet = CharSet.Auto)]
        static extern bool PathAppend([In, Out] StringBuilder pszPath, string pszMore);
        public static int GetFileLinkCount(string filepath) {
            int result = 0;
            using(SafeFileHandle handle = CreateFile(filepath, FileAccess.Read,
              FileShare.Read, IntPtr.Zero, FileMode.Open, FileAttributes.Normal,
              IntPtr.Zero)) {
                if (handle.IsInvalid)
                    Marshal.ThrowExceptionForHR(Marshal.GetHRForLastWin32Error());
                BY_HANDLE_FILE_INFORMATION fileInfo = new BY_HANDLE_FILE_INFORMATION();
                if (GetFileInformationByHandle(handle, out fileInfo))
                  result = (int)fileInfo.NumberOfLinks;
                return result;
            }
        }
        public static string[] GetFileSiblingHardLinks(string filepath) {
            List<string> result = new List<string>();
            uint stringLength = 256;
            StringBuilder sb = new StringBuilder(256);
            GetVolumePathName(filepath, sb, stringLength);
            string volume = sb.ToString();
            sb.Length = 0; stringLength = 256;
            IntPtr findHandle = FindFirstFileNameW(filepath, 0,
                ref stringLength, sb);
            if (findHandle.ToInt32() != -1) {
                do {
                    StringBuilder pathSb = new StringBuilder(volume, 256);
                    PathAppend(pathSb, sb.ToString());
                    string s = pathSb.ToString();
                    if(!s.Equals(filepath, StringComparison.InvariantCultureIgnoreCase))
                        result.Add(s);
                    sb.Length = 0; stringLength = 256;
                } while (FindNextFileNameW(findHandle, ref stringLength, sb));
                FindClose(findHandle);
                return result.ToArray();
            }
            return null;
        }
    }
"@
  } catch {
    if ($Verbose) {
      Write-Host $_.Exception.Message -ForegroundColor Cyan
    }
  }
}

# Load Kernel32 Type and then invoke our main method.
addKernel32Type
main
