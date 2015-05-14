@echo off

set node_dir=E:\Repos\my_node-msft
set release_dir=E:\Repos\my_ntvsiot\Setup\NodejsUwpFiles

@rem arm

@rem build node.dll
call "%node_dir%\vcbuild.bat" arm chakra uwp-dll withoutssl
cd %~dp0
@rem build nodeuwp.dll
set WindowsSdkDir=C:\Program Files (x86)\Windows Kits\8.2\
msbuild nodeuwp.sln /p:configuration=release /p:platform=arm

@rem copy to release directory
xcopy /y /f /i "%node_dir%\Release\node.dll" "%release_dir%\ARM"
xcopy /y /f /i %~dp0\ARM\release\nodeuwp\nodeuwp.dll "%release_dir%\ARM"

@rem x86x

@rem build node.dll
call "%node_dir%\vcbuild.bat" x86 chakra uwp-dll withoutssl
cd %~dp0
@rem build nodeuwp.dll
set WindowsSdkDir=C:\Program Files (x86)\Windows Kits\8.2\
msbuild nodeuwp.sln /p:configuration=release /p:platform=x86

@rem copy to release directory
xcopy /y /f /i "%node_dir%\Release\node.dll" "%release_dir%\x86"
xcopy /y /f /i %~dp0\release\nodeuwp\nodeuwp.dll "%release_dir%\x86"

@rem x64

@rem build node.dll
call "%node_dir%\vcbuild.bat" x64 chakra uwp-dll withoutssl
cd %~dp0
@rem build nodeuwp.dll
set WindowsSdkDir=C:\Program Files (x86)\Windows Kits\8.2\
msbuild nodeuwp.sln /p:configuration=release /p:platform=x64

@rem copy to release directory
xcopy /y /f /i "%node_dir%\Release\node.dll" "%release_dir%\x64"
xcopy /y /f /i %~dp0\x64\release\nodeuwp\nodeuwp.dll "%release_dir%\x64"

:end