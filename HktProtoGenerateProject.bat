@echo off
set "ENGINE_PATH=E:\WS\UE_5.6\Engine"
set "UPROJECT_PATH=%~dp0HktProto.uproject"

echo Generating Visual Studio Project Files...

"%ENGINE_PATH%\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="%UPROJECT_PATH%" -game -rocket -progress

pause