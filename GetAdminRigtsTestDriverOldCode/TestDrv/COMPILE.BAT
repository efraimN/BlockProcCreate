set MyPrjectdirectory=C:\Projects\DialerVistaXp\XpVistaCommon\DialerApiDriver

set myddkroot=c:\WINDDK\6000
@rem set myddkroot=c:\WINDDK\5384
@rem set myddkroot=c:\WINDDK\2600.1106
@rem set myddkroot=c:\WINDDK\3790.1830






rem @echo off
rem set MSDEVDIR="C:\Program Files\Microsoft Visual Studio\Common\MSDev98\Bin"

c:
cd  %myddkroot%\bin
@rem call setenv.bat  %myddkroot% checked %1
@rem call setenv.bat  %myddkroot% checked AMD64 %1
@rem call setenv.bat  %myddkroot% fre IA64 %1

call setenv.bat  %myddkroot% fre %1
@rem call setenv.bat  %myddkroot% fre AMD64 %1

@rem @echo off
c:

set DRIVER_COMPILE_FLAGS=FAcs /Wp64 /Fa$(O)\ %2

cd %MyPrjectdirectory%
set MYDIR=%1
set MYBASEDIR=%MyPrjectdirectory%
if %CPU%==x86 set MCPU=i386
if %CPU%==x64 set MCPU=amd64
if "%3"=="clean" (
 rd %1\%MCPU% /s /q
)
if not exist %1\%MCPU% (
md %1\%MCPU% > nul:
)

@echo on
%myddkroot%\bin\x86\build -ei -cFZwbgu -jpath %1\%MCPU% %1
@echo off


if ERRORLEVEL 1 goto err

goto end

:err
@echo *********** ERROR!!!! Errors found. Look at the logs!!!. *************
:end

cd %MyPrjectdirectory%

exit
rem ed nt!kd_default_mask