@echo off
REM --------------------------------------------------
REM GenPacket.bat
REM Usage: 이 파일이 있는 디렉터리에서 더블클릭 또는 cmd에서 실행
REM --------------------------------------------------

REM 현재 스크립트 위치
set SCRIPT_DIR=%~dp0

REM 솔루션 디렉토리
for %%I in ("%SCRIPT_DIR%..\..") do set SOLUTION_DIR=%%~fI

REM protoc.exe 경로
set PROTOC=%SOLUTION_DIR%\vcpkg_installed\x64-windows-static\tools\protobuf\protoc.exe

REM .proto 파일들이 있는 루트 디렉터리
set PROTO_ROOT=%SOLUTION_DIR%\Common\Protocol

REM 임시 출력 디렉터리
set OUT_DIR=%SCRIPT_DIR%Build

REM 최종 복사 대상 디렉터리들
set COPY_DIRS="%SOLUTION_DIR%\GameServer\Network\Protocol" "%SOLUTION_DIR%\DummyClient\Network\Protocol"

REM 1) 출력 폴더 초기화
if exist "%OUT_DIR%" (
    rd /s /q "%OUT_DIR%"
)
mkdir "%OUT_DIR%"

echo ============================================
echo Generating C++ sources from .proto files...
echo Proto root: %PROTO_ROOT%
echo Temp out  : %OUT_DIR%
echo Targets   : %COPY_DIRS%
echo Using protoc: %PROTOC%
echo ============================================

REM 2) .proto→CPP 코드 생성
for /R "%PROTO_ROOT%" %%F in (*.proto) do (
    echo [PROTO] Compiling %%~fF
    "%PROTOC%" -I="%PROTO_ROOT%" --cpp_out="%OUT_DIR%" "%%~fF"
    if errorlevel 1 (
        echo [ERROR] Failed to compile %%~fF
        exit /b 1
    )
)

REM 3) 생성된 파일을 각 복사 대상에 배포
for %%D in (%COPY_DIRS%) do (
    echo [COPY] -> %%~D
    xcopy "%OUT_DIR%\*" "%%~D\" /E /Y /I
    if errorlevel 1 (
        echo [WARNING] Copy to %%~D may have failed.
    )
)

REM 4) 임시 출력 폴더 정리
echo [CLEAN] Removing temp dir %OUT_DIR%
rd /s /q "%OUT_DIR%"

REM 5) PacketGenerator.py 실행
echo ============================================
echo Generating packet code...
echo ============================================
python "%SCRIPT_DIR%PacketGenerator.py" --client_name "DummyClient" --client_pkt_prefix "C2S" --server_name "GameServer" --server_pkt_prefix "S2C"
if errorlevel 1 (
    echo [ERROR] Failed to run PacketGenerator.py
    exit /b 1
)

echo All done!
exit /b 0
