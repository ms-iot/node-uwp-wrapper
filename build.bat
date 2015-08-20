@echo off

@rem This batch file builds and signs binaries that are deployed by NTVS IoT Extension (https://github.com/ms-iot/ntvsiot).
@rem List of binaries:
@rem 1. node.dll (https://github.com/Microsoft/node - in *uwp branches)
@rem 2. nodeuwp.dll (https://github.com/ms-iot/node-uwp-wrapper)
@rem 3. uwp.node (https://github.com/Microsoft/node-uwp)

@rem To build successfully, clones of all the above projects are required.

@rem Usage:
@rem build.bat [x86 | ARM | x64] [builduwpaddon] [copyrelease] [sign] [onlysign]

@rem If processor architecture isn't specified all architectures will be built
@rem To use the 'sign' option, builduwpaddon and copyrelease need to be included
@rem onlysign can be the only option provided if the release binaries already exist (release_dir needs to be set)

@rem Example:
@rem set node_dir=c:\repos\node_clone
@rem set uwpaddon_dir=c:\repos\uwpaddon_clone
@rem set release_dir=c:\release
@rem build.bat ARM builduwpaddon copyrelease

@rem The result of the command above will be:
@rem c:\release\arm\node.dll
@rem               \nodeuwp.dll
@rem 			   \uwp.node

if /i "%1"=="onlysign" (
  if not defined release_dir (
    echo Error: release_dir needs to be set when using onlysign.
    goto end
  )
  echo Signing ARM binaries...
  powershell -command "& { . .\sign.ps1; begin_sign_files -files 'node.dll','nodeuwp.dll','uwp.node' -bindir '%release_dir%\ARM' -outdir '%release_dir%\ARM\Signed' -approvers 'jinglou','sitani' -projectName 'NTVS IoT' -projectUrl 'https://github.com/ms-iot/ntvsiot' -jobDescription 'Node.js (Chakra) ARM binaries for NTVS IoT' -jobKeywords 'NTVS', 'Node.js', 'IoT' -certificates 'authenticode'}"
  echo Signing x86 binaries...
  powershell -command "& { . .\sign.ps1; begin_sign_files -files 'node.dll','nodeuwp.dll','uwp.node' -bindir '%release_dir%\x86' -outdir '%release_dir%\x86\Signed' -approvers 'jinglou','sitani' -projectName 'NTVS IoT' -projectUrl 'https://github.com/ms-iot/ntvsiot' -jobDescription 'Node.js (Chakra) x86 binaries for NTVS IoT' -jobKeywords 'NTVS', 'Node.js', 'IoT' -certificates 'authenticode'}"
  echo Signing x64 binaries...
  powershell -command "& { . .\sign.ps1; begin_sign_files -files 'node.dll','nodeuwp.dll','uwp.node' -bindir '%release_dir%\x64' -outdir '%release_dir%\x64\Signed' -approvers 'jinglou','sitani' -projectName 'NTVS IoT' -projectUrl 'https://github.com/ms-iot/ntvsiot' -jobDescription 'Node.js (Chakra) x64 binaries for NTVS IoT' -jobKeywords 'NTVS', 'Node.js', 'IoT' -certificates 'authenticode'}"
  goto end
)

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
if /i "%1"=="debug"           set debug=1

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
if defined debug (
  call "%node_dir%\vcbuild.bat" arm chakra nosign uwp-dll openssl-no-asm debug
)
pushd %batch_dir%
@rem build nodeuwp.dll
set WindowsSdkDir=%programfiles(x86)%\Windows Kits\10\
msbuild nodeuwp.sln /p:configuration=release /p:platform=arm
if defined debug (
  msbuild nodeuwp.sln /p:configuration=debug /p:platform=arm
)
@rem build uwp.node
if defined builduwpaddon (
  pushd %uwpaddon_dir%
  node.exe %node_dir%\deps\npm\node_modules\node-gyp\bin\node-gyp.js rebuild --nodedir=%node_dir% --node_win_onecore --target_arch=arm
  if defined debug (
    node.exe %node_dir%\deps\npm\node_modules\node-gyp\bin\node-gyp.js build --nodedir=%node_dir% --node_win_onecore --target_arch=arm --debug
  )
)
@rem copy to release directory
if defined copyrelease (
  echo D | xcopy /y /f "%node_dir%\Release\node.*" "%release_dir%\Release\ARM"
  echo D | xcopy /y /f "%batch_dir%\ARM\release\nodeuwp\nodeuwp.*" "%release_dir%\Release\ARM"
  if defined builduwpaddon (
    echo D | xcopy /y /f "%uwpaddon_dir%\build\release\uwp.*" "%release_dir%\Release\ARM"
  )
  if defined debug (
    echo D | xcopy /y /f "%node_dir%\Debug\node.*" "%release_dir%\Debug\ARM"
    echo D | xcopy /y /f "%node_dir%\Debug\*.pdb" "%release_dir%\Debug\ARM"
    echo D | xcopy /y /f "%batch_dir%\ARM\debug\nodeuwp\nodeuwp.*" "%release_dir%\Debug\ARM"
    if defined builduwpaddon (
      echo D | xcopy /y /f "%uwpaddon_dir%\build\debug\uwp.*" "%release_dir%\Debug\ARM"
    )
  )
)

@rem sign the binaries
if defined sign (
  echo Signing ARM binaries...
  powershell -command "& { . .\sign.ps1; begin_sign_files -files 'node.dll','nodeuwp.dll','uwp.node' -bindir '%release_dir%\ARM' -outdir '%release_dir%\ARM\Signed' -approvers 'jinglou','sitani' -projectName 'NTVS IoT' -projectUrl 'https://github.com/ms-iot/ntvsiot' -jobDescription 'Node.js (Chakra) ARM binaries for NTVS IoT' -jobKeywords 'NTVS', 'Node.js', 'IoT' -certificates 'authenticode'}"
)

pushd %batch_dir%
if not defined buildall goto end

:x86
@rem build node.dll
call "%node_dir%\vcbuild.bat" x86 chakra nosign uwp-dll
if defined debug (
  call "%node_dir%\vcbuild.bat" x86 chakra nosign uwp-dll debug
)
pushd %batch_dir%
@rem build nodeuwp.dll
set WindowsSdkDir=%programfiles(x86)%\Windows Kits\10\
msbuild nodeuwp.sln /p:configuration=release /p:platform=x86
if defined debug (
  msbuild nodeuwp.sln /p:configuration=debug /p:platform=x86
)
@rem build uwp.node
if defined builduwpaddon (
  pushd %uwpaddon_dir%
  node.exe %node_dir%\deps\npm\node_modules\node-gyp\bin\node-gyp.js rebuild --nodedir=%node_dir% --node_win_onecore --target_arch=x86
  if defined debug (
    node.exe %node_dir%\deps\npm\node_modules\node-gyp\bin\node-gyp.js build --nodedir=%node_dir% --node_win_onecore --target_arch=x86 --debug
  )
)
@rem copy to release directory
if defined copyrelease (
  echo D | xcopy /y /f "%node_dir%\Release\node.*" "%release_dir%\Release\x86"
  echo D | xcopy /y /f "%batch_dir%\release\nodeuwp\nodeuwp.*" "%release_dir%\Release\x86"
  if defined builduwpaddon (
    echo D | xcopy /y /f "%uwpaddon_dir%\build\release\uwp.*" "%release_dir%\Release\x86"
  )
  if defined debug (
    echo D | xcopy /y /f "%node_dir%\Debug\node.*" "%release_dir%\Debug\x86"
    echo D | xcopy /y /f "%node_dir%\Debug\*.pdb" "%release_dir%\Debug\x86"
    echo D | xcopy /y /f "%batch_dir%\debug\nodeuwp\nodeuwp.*" "%release_dir%\Debug\x86"
    if defined builduwpaddon (
      echo D | xcopy /y /f "%uwpaddon_dir%\build\debug\uwp.*" "%release_dir%\Debug\x86"
    )
  )
)

@rem sign the binaries
if defined sign (
  echo Signing x86 binaries...
  powershell -command "& { . .\sign.ps1; begin_sign_files -files 'node.dll','nodeuwp.dll','uwp.node' -bindir '%release_dir%\x86' -outdir '%release_dir%\x86\Signed' -approvers 'jinglou','sitani' -projectName 'NTVS IoT' -projectUrl 'https://github.com/ms-iot/ntvsiot' -jobDescription 'Node.js (Chakra) x86 binaries for NTVS IoT' -jobKeywords 'NTVS', 'Node.js', 'IoT' -certificates 'authenticode'}"
)

pushd %batch_dir%
if not defined buildall goto end

:x64
@rem build node.dll
call "%node_dir%\vcbuild.bat" x64 chakra nosign uwp-dll
if defined debug (
  call "%node_dir%\vcbuild.bat" x64 chakra nosign uwp-dll debug
)
pushd %batch_dir%
@rem build nodeuwp.dll
set WindowsSdkDir=%programfiles(x86)%\Windows Kits\10\
msbuild nodeuwp.sln /p:configuration=release /p:platform=x64
if defined debug (
  msbuild nodeuwp.sln /p:configuration=debug /p:platform=x64
)
@rem build uwp.node
if defined builduwpaddon (
  pushd %uwpaddon_dir%
  node.exe %node_dir%\deps\npm\node_modules\node-gyp\bin\node-gyp.js rebuild --nodedir=%node_dir% --node_win_onecore --target_arch=x64
  if defined debug (
    node.exe %node_dir%\deps\npm\node_modules\node-gyp\bin\node-gyp.js build --nodedir=%node_dir% --node_win_onecore --target_arch=x64 --debug
  )
)
@rem copy to release directory
if defined copyrelease (
  echo D | xcopy /y /f "%node_dir%\Release\node.*" "%release_dir%\Release\x64"
  echo D | xcopy /y /f "%batch_dir%\x64\release\nodeuwp\nodeuwp.*" "%release_dir%\Release\x64"
  if defined builduwpaddon (
    echo D | xcopy /y /f "%uwpaddon_dir%\build\release\uwp.*" "%release_dir%\Release\x64"
  )
  if defined debug (
    echo D | xcopy /y /f "%node_dir%\Debug\node.*" "%release_dir%\Debug\x64"
    echo D | xcopy /y /f "%node_dir%\Debug\*.pdb" "%release_dir%\Debug\x64"
    echo D | xcopy /y /f "%batch_dir%\x64\debug\nodeuwp\nodeuwp.dll" "%release_dir%\Debug\x64"
    if defined builduwpaddon (
      echo D | xcopy /y /f "%uwpaddon_dir%\build\debug\uwp.*" "%release_dir%\Debug\x64"
    )
  )
)

@rem sign the binaries
if defined sign (
  echo Signing x64 binaries...
  powershell -command "& { . .\sign.ps1; begin_sign_files -files 'node.dll','nodeuwp.dll','uwp.node' -bindir '%release_dir%\x64' -outdir '%release_dir%\x64\Signed' -approvers 'jinglou','sitani' -projectName 'NTVS IoT' -projectUrl 'https://github.com/ms-iot/ntvsiot' -jobDescription 'Node.js (Chakra) x64 binaries for NTVS IoT' -jobKeywords 'NTVS', 'Node.js', 'IoT' -certificates 'authenticode'}"
)

pushd %batch_dir%

:end