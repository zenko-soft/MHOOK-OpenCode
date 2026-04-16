@echo off
"C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\MSBuild.exe" "C:\Programs\mhook\source mhook\MHook64.sln" /p:Configuration=Release /p:Platform=x64 /verbosity:minimal
"C:\Programs\mhook\source mhook\upx_temp\upx-4.2.4-win64\upx.exe" -9 "C:\Programs\mhook\source mhook\x64\Release\MHook64.exe"
