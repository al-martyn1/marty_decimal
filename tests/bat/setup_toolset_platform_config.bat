@rem %1 - toolset name, eg msvc2017
@rem %2 - platform name, eg x86 or x64
@rem %3 - configuration name, eg Debug or Release

@if "%1" == "" goto USAGE_EXIT
@if "%2" == "" goto USAGE_EXIT
@if "%3" == "" goto USAGE_EXIT
@if "%OUTPUT_ROOT%" == "" goto USAGE_EXIT

@if "%1"=="msvc2017" goto TOOLSET_MSVC2017
@echo Invalid toolset specified - %1
@rem The system cannot find the path specified. Indicates that the specified path cannot be found.
call :USAGE && exit /B 3

:TOOLSET_MSVC2017
@set TOOLSET=%1
@goto CHECK_PALTFORM



:CHECK_PALTFORM

@if "%2"=="x86" goto PALTFORM_X86
@if "%2"=="x64" goto PALTFORM_X64
@echo Invalid platform specified - %2
@rem The system cannot find the path specified. Indicates that the specified path cannot be found.
call :USAGE && exit /B 3

:PALTFORM_X86
@set QT_PLATFORM_SUBPATH=%TOOLSET%
@goto PLATFORM_CHECK_DONE

:PALTFORM_X64
@set QT_PLATFORM_SUBPATH=%TOOLSET%_64
@goto PLATFORM_CHECK_DONE

:PLATFORM_CHECK_DONE
@set QT_PLATFORM_ROOT=%QTROOT%\%QT_PLATFORM_SUBPATH%
@set PLATFORM=%2
@set PLATFORM_OUTPUT_ROOT=%OUTPUT_ROOT%\%TOOLSET%\%PLATFORM%


@if "%3"=="Release" goto CONFIGURATION_RELEASE
@if "%3"=="Debug"   goto CONFIGURATION_DEBUG
@echo Invalid configuration specified - %3
@rem The system cannot find the path specified. Indicates that the specified path cannot be found.
call :USAGE && exit /B 3

:CONFIGURATION_DEBUG
@set WINDEPLOYQT_CONFIGURATION_OPTION=--debug
@goto CONFIGURATION_COMMON

:CONFIGURATION_RELEASE
@set WINDEPLOYQT_CONFIGURATION_OPTION=--release
@goto CONFIGURATION_COMMON

:CONFIGURATION_COMMON
@set CONFIGURATION=%3
@set OUTDIR=%PLATFORM_OUTPUT_ROOT%\%CONFIGURATION%
@set WINDEPLOYQT="%QT_PLATFORM_ROOT%\bin\windeployqt.exe" %WINDEPLOYQT_CONFIGURATION_OPTION%
@set DEPLOY_PATH=%DEPLOY_ROOT%\%TOOLSET%\%PLATFORM%\%CONFIGURATION%
@goto END


:USAGE
@echo Usage: setup_toolset_platform_config.bat toolset platform configuration
@echo Where
@echo   toolset       - Toolset Name, eg msvc2017
@echo   platform      - Platform Name, eg x86 or x64
@echo   configuration - Configuration Name, eg Debug or Release
@echo OUTPUT_ROOT environment variable must set up before calling this bat file
@exit /B

:USAGE_EXIT
@call :USAGE
@exit /B 1

:END
@exit /B 0
