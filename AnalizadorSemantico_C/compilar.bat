@echo off
setlocal
cd /d "%~dp0"
where gcc >nul 2>nul
if errorlevel 1 (
    if exist "C:\msys64\ucrt64\bin\gcc.exe" (
        set "PATH=C:\msys64\ucrt64\bin;%PATH%"
    ) else (
        echo No se encontro GCC. Instala MinGW GCC o agrega su carpeta bin al PATH.
        pause
        exit /b 1
    )
)
gcc main.c analizador.c -o AnalizadorSemantico.exe -mwindows -Wall -Wextra -std=c11 -lgdi32 -luser32
if errorlevel 1 (
    echo Error de compilacion. Instala MinGW GCC o compila desde Code::Blocks.
    pause
    exit /b 1
)
echo Compilacion exitosa: AnalizadorSemantico.exe
pause
