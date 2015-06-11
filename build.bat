@echo off

if not defined node_dir (
  echo Error: set node_dir to the path of your Node.js Git clone.
  goto end
)

if defined builduwpaddon (
  if not defined uwpaddon_dir (
    echo Error: To use builduwpaddon option, set uwpaddon_dir to the path of your uwp addon Git clone
    goto end
  ) 
)

set batch_dir=%~dp0

set copyrelease=
set target_arch=
set target_arch=
set target_arch=
set buildall=
set builduwpaddon=

:next-arg
if "%1"=="" goto args-done
if /i "%1"=="copyrelease"     set copyrelease=1
if /i "%1"=="arm"             set target_arch=arm
if /i "%1"=="x86"             set target_arch=x86
if /i "%1"=="x64"             set target_arch=x64
if /i "%1"=="builduwpaddon"   set builduwpaddon=1

:arg-ok
shift
goto next-arg

:args-done

if defined copyrelease (
  if not defined release_dir (
    echo Error: release_dir needs to be set when using copyrelease.
    goto end
  )
)

if "%target_arch%"=="arm" goto arm
if "%target_arch%"=="x86" goto x86
if "%target_arch%"=="x64" goto x64

set buildall=1

:arm
@rem build node.dll
call "%node_dir%\vcbuild.bat" arm chakra nosign uwp-dll openssl-no-asm
pushd %batch_dir%
@rem build nodeuwp.dll
set WindowsSdkDir=%programfiles(x86)%\Windows Kits\10\
msbuild nodeuwp.sln /p:configuration=release /p:platform=arm
@rem build uwp.node
if defined builduwpaddon (
  pushd %uwpaddon_dir%
  node.exe %node_dir%\deps\npm\node_modules\node-gyp\bin\node-gyp.js rebuild --nodedir=%node_dir% --node_win_onecore --target_arch=arm
)
@rem copy to release directory
if defined copyrelease (
  echo D | xcopy /y /f "%node_dir%\Release\node.dll" "%release_dir%\ARM"
  echo D | xcopy /y /f "%batch_dir%\ARM\release\nodeuwp\nodeuwp.dll" "%release_dir%\ARM"
  if defined builduwpaddon (
    echo D | xcopy /y /f "%uwpaddon_dir%\build\release\uwp.node" "%release_dir%\ARM"
  )
)
pushd %batch_dir%
if not defined buildall goto end

:x86
@rem build node.dll
call "%node_dir%\vcbuild.bat" x86 chakra nosign uwp-dll
pushd %batch_dir%
@rem build nodeuwp.dll
set WindowsSdkDir=%programfiles(x86)%\Windows Kits\10\
msbuild nodeuwp.sln /p:configuration=release /p:platform=x86
@rem build uwp.node
if defined builduwpaddon (
  pushd %uwpaddon_dir%
  node.exe %node_dir%\deps\npm\node_modules\node-gyp\bin\node-gyp.js rebuild --nodedir=%node_dir% --node_win_onecore --target_arch=x86
)
@rem copy to release directory
if defined copyrelease (
  echo D | xcopy /y /f "%node_dir%\Release\node.dll" "%release_dir%\x86"
  echo D | xcopy /y /f "%batch_dir%\release\nodeuwp\nodeuwp.dll" "%release_dir%\x86"
  if defined builduwpaddon (
    echo D | xcopy /y /f "%uwpaddon_dir%\build\release\uwp.node" "%release_dir%\x86"
  )
)
pushd %batch_dir%
if not defined buildall goto end

:x64
@rem build node.dll
call "%node_dir%\vcbuild.bat" x64 chakra nosign uwp-dll
pushd %batch_dir%
@rem build nodeuwp.dll
set WindowsSdkDir=%programfiles(x86)%\Windows Kits\10\
msbuild nodeuwp.sln /p:configuration=release /p:platform=x64
@rem build uwp.node
if defined builduwpaddon (
  pushd %uwpaddon_dir%
  node.exe %node_dir%\deps\npm\node_modules\node-gyp\bin\node-gyp.js rebuild --nodedir=%node_dir% --node_win_onecore --target_arch=x64
)
@rem copy to release directory
if defined copyrelease (
  echo D | xcopy /y /f "%node_dir%\Release\node.dll" "%release_dir%\x64"
  echo D | xcopy /y /f "%batch_dir%\x64\release\nodeuwp\nodeuwp.dll" "%release_dir%\x64"
  if defined builduwpaddon (
    echo D | xcopy /y /f "%uwpaddon_dir%\build\release\uwp.node" "%release_dir%\x64"
  )
)
pushd %batch_dir%

:end