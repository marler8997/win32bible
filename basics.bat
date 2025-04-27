mkdir out
cl /Feout\basics.exe /Foout\ /DUNICODE /D_UNICODE src/basics.c src/GetMsgName.c
@if %errorlevel% neq 0 (exit /b %errorlevel%)
out\basics.exe
