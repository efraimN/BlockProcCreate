SETLOCAL ENABLEDELAYEDEXPANSION
@echo on
rem setlocal

REM %%1 $(OutDir)$(TargetName)$(TargetExt)


IF DEFINED SIGN_COMMAND (
   IF "%BUILD_TYPE%" EQU "Release" (
        IF "!SIGN_COMMAND:""=!" EQU "none" (
          exit /b 0
        )
    )
) else (
   set SIGN_COMMAND= /f %~dp0CARoot.pfx /fd sha256
)


set Tempp=%~dp0signtool.exe sign /v %SIGN_COMMAND% %*

echo %Tempp%
%Tempp% 

endlocal
exit /b %errorlevel%

