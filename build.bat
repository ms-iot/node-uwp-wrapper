@echo off

if not defined node_dir (
  echo Error: set node_dir to the path of your Node.js clone.
  goto end
)

set batch_dir=%~dp0

set copyrelease=
set target_arch=
set target_arch=
set target_arch=
set buildall=

:next-arg
if "%1"=="" goto args-done
if /i "%1"=="copyrelease" set copyrelease=1
if /i "%1"=="arm"         set target_arch=arm
if /i "%1"=="x86"         set target_arch=x86
if /i "%1"=="x64"         set target_arch=x64

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
call "%node_dir%\vcbuild.bat" arm chakra uwp-dll withoutssl
pushd %batch_dir%
@rem build nodeuwp.dll
set WindowsSdkDir=%programfiles(x86)%\Windows Kits\10\
msbuild nodeuwp.sln /p:configuration=release /p:platform=arm

@rem copy to release directory
if defined copyrelease (
  echo D | xcopy /y /f "%node_dir%\Release\node.dll" "%release_dir%\ARM"
  echo D | xcopy /y /f "%~dp0\ARM\release\nodeuwp\nodeuwp.dll" "%release_dir%\ARM"
)
if not defined buildall goto end

:x86
@rem build node.dll
call "%node_dir%\vcbuild.bat" x86 chakra uwp-dll withoutssl
pushd %batch_dir%
@rem build nodeuwp.dll
set WindowsSdkDir=%programfiles(x86)%\Windows Kits\10\
msbuild nodeuwp.sln /p:configuration=release /p:platform=x86

@rem copy to release directory
if defined copyrelease (
  echo D | xcopy /y /f "%node_dir%\Release\node.dll" "%release_dir%\x86"
  echo D | xcopy /y /f "%~dp0\release\nodeuwp\nodeuwp.dll" "%release_dir%\x86"
)
if not defined buildall goto end

:x64
@rem build node.dll
call "%node_dir%\vcbuild.bat" x64 chakra uwp-dll withoutssl
pushd %batch_dir%
@rem build nodeuwp.dll
set WindowsSdkDir=%programfiles(x86)%\Windows Kits\10\
msbuild nodeuwp.sln /p:configuration=release /p:platform=x64

@rem copy to release directory
if defined copyrelease (
  echo D | xcopy /y /f "%node_dir%\Release\node.dll" "%release_dir%\x64"
  echo D | xcopy /y /f "%~dp0\x64\release\nodeuwp\nodeuwp.dll" "%release_dir%\x64"
)

:end