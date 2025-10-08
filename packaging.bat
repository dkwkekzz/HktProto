@echo off
REM ===== 변수 설정 =====
set ENGINE_PATH=C:\WS\DEV\UnrealEngine
set PROJECT_PATH=C:\WS\DEV\UE5\HktProto
set PLATFORM=Win64
set CONFIG=DebugGame

REM ===== 경로 확인 출력 (디버깅용) =====
echo ENGINE_PATH: %ENGINE_PATH%
echo PROJECT_PATH: %PROJECT_PATH%
echo PLATFORM: %PLATFORM%
echo CONFIG: %CONFIG%
echo.

REM ===== Unreal Automation Tool 실행 =====
call "%ENGINE_PATH%\Engine\Build\BatchFiles\RunUAT.bat" ^
    BuildCookRun ^
    -project="%PROJECT_PATH%\HktProto.uproject" ^
    -noP4 ^
    -platform=%PLATFORM% ^
    -clientconfig=%CONFIG% ^
    -cook ^
    -allmaps ^
    -build ^
    -stage ^
    -pak ^
    -archive ^
    -archivedirectory="%PROJECT_PATH%\Packages\%PLATFORM%_%CONFIG%"

pause
