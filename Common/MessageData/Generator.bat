@echo off
rem 현재 디렉토리
set "BASE_DIR=%~dp0"
rem 생성된 코드가 저장될 디렉토리
set "GENERATED_DIR=%BASE_DIR%Generated\"
rem 스키마 디렉토리
set "SCHEMA_DIR=%BASE_DIR%Schema\"
rem flatc 디렉토리
set "FLATC_DIR=..\..\vcpkg_installed\x64-windows\tools\flatbuffers\"
rem 스키마를 컴파일하여 코드 생성
%FLATC_DIR%flatc --cpp -o %GENERATED_DIR% %SCHEMA_DIR%Common.fbs
%FLATC_DIR%flatc --cpp -o %GENERATED_DIR% %SCHEMA_DIR%Server.fbs
%FLATC_DIR%flatc --cpp -o %GENERATED_DIR% %SCHEMA_DIR%Client.fbs
IF ERRORLEVEL 1 PAUSE
