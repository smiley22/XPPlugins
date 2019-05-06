# Creates a distributable zip archive with binaries and data
Add-Type -AssemblyName System.IO.Compression -ErrorAction Stop

function main($plugin, $version) {
  if(!$plugin -or !$version) {
    error "Missing parameter(s). Syntax: ./package <plugin> <version>"
  }
  if (![IO.Directory]::Exists("$PSScriptRoot\$plugin")) {
    error "Could not find directory $plugin"
  }
  if (![IO.File]::Exists("$PSScriptRoot\x64\Release\$plugin.dll")) {
    error "Could not find binary $plugin.dll"
  }
  createArchive $plugin $version
  Write-Host "All done"
}

function createArchive($plugin, [string] $version) {
  $zip = "$PSScriptRoot\${plugin}_v${version}.zip"
  __using($fs = New-Object IO.FileStream $zip, 'Create') {
    __using($ar = New-Object IO.Compression.ZipArchive $fs, 'Create') {
      Write-Host "Creating archive " -NoNewLine
      Write-Host $zip -ForegroundColor Yellow
      $file = "$PSScriptRoot\x64\Release\$plugin.dll"
      addFile $ar "$plugin\64\win.xpl" $file
      $file = "$dir\mac.xpl"
      if ([IO.File]::Exists($file)) {
        addFile $ar "$plugin\64\mac.xpl" $file
      } else {
        Write-Host "Warning, no mac.xpl file found" -ForegroundColor Magenta
      }
      $file = "$dir\README.md"
      if ([IO.File]::Exists($file)) {
        addFile $ar "$plugin\Readme.txt" $file
      }
      $file = "$dir\settings.ini"
      if ([IO.File]::Exists($file)) {
        addFile $ar "$plugin\settings.ini" $file
      }
      $data = "$dir\data"
      if ([IO.Directory]::Exists($data)) {
        # Get all files in data
        $files = [IO.Directory]::GetFiles($data, "*.*", 'AllDirectories')
        foreach ($f in $files) {
          $t = $f.Replace($PSScriptRoot, "").TrimStart('\');
          addFile $ar $t $f
        }
      }
    }
  }
}

function addFile($archive, $path, $file) {
  $entry = $archive.CreateEntry($path)
  __using($s = $entry.Open()) {
    __using($fs = New-Object IO.FileStream $file, 'Open') {
      Write-Host " - Adding " -NoNewLine
      Write-Host $path -ForegroundColor Green -NoNewLine
      Write-Host " from " -NoNewLine
      Write-Host $file -ForegroundColor Cyan
      $fs.CopyTo($s)
    }
  }
}

function error($msg) {
  Write-Host "[" -NoNewLine
  Write-Host "Error" -ForegroundColor Red -NoNewLine
  Write-Host "] " -NoNewLine
  Write-Host $msg
  exit
}

# Emulate c# 'using' statement...
function __using {
  param (
    [IDisposable] $disposable,
    [ScriptBlock] $scriptBlock
  )
  try {
    & $scriptBlock
  } finally {
    if ($disposable -ne $null) {
      $disposable.Dispose()
    }
  }
}

main @args
