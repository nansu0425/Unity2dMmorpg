@echo off
rem 현재 디렉토리
set "BASE_DIR=%~dp0"
rem flatc 디렉토리
set "FLATC_DIR=..\..\vcpkg_installed\x64-windows\tools\flatbuffers\"
rem 스키마를 컴파일하여 코드 생성
%FLATC_DIR%flatc --cpp -o %BASE_DIR% %BASE_DIR%Common.fbs
%FLATC_DIR%flatc --cpp -o %BASE_DIR% %BASE_DIR%Server.fbs
%FLATC_DIR%flatc --cpp -o %BASE_DIR% %BASE_DIR%Client.fbs
IF ERRORLEVEL 1 PAUSE
