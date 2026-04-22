@echo off
REM Batch script to copy cpacs test files to build directory

SET "SRC_DIR=tests\unittests\TestData"
SET "DEST=build-vs\tests\unittests\TestData"

REM Create destination directory if it doesn't exist
IF NOT EXIST "%DEST%" (
    echo Creating directory %DEST%
    mkdir "%DEST%"
)

REM Copy files
for %%F in (
    simpletest-decks.cpacs.xml
) do (
    echo Copying "%SRC_DIR%\%%F" to "%DEST%"
    copy "%SRC_DIR%\%%F" "%DEST%"
    IF ERRORLEVEL 1 (
        echo Failed to copy %%F
        exit /b 1
    )
)

echo All files copied successfully.
