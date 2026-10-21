# Retomar en otra PC

Estado guardado el 2026-10-21 en la rama `buck-boost-pruebas` del fork
`https://github.com/matiaschamu/cheali-charger.git`.

```powershell
git clone --branch buck-boost-pruebas https://github.com/matiaschamu/cheali-charger.git
cd cheali-charger
```

Leer `AGENTS.md` y `utils/CMS32L051_flash_tools/README.md` antes de continuar.
El port sigue en validación de banco: compilar no demuestra funcionamiento.
La bitácora contiene las evidencias y pendientes de ADC, PWM y calibración.

## Herramientas y rutas

Instalar Git, CMake, MinGW Make y Arm GNU Toolchain (GCC/G++/GDB/objdump/size).
Para depurar: VS Code, extensiones recomendadas y xPack OpenOCD 0.12.0-7.
La configuración guardada usa estas ubicaciones del equipo anterior:

- CMake: `C:/Program Files/CMake/bin/cmake.exe`.
- MinGW: `C:/GCC/mingw64/bin/mingw32-make.exe`.
- Arm GNU: `C:/Program Files/Arm/GNU Toolchain mingw-w64-x86_64-arm-none-eabi/bin`.
- OpenOCD: `%USERPROFILE%/Downloads/openocd/xpack-openocd-0.12.0-7/bin/openocd.exe`.

Ajustar `.vscode/tasks.json`, `.vscode/launch.json` y
`utils/CMS32L051_flash_tools/flash_debug_safe.ps1` si las rutas difieren.
Agregar las herramientas al PATH según sea necesario. Generar builds nuevos;
no trasladar `CMakeCache.txt` ni objetos de compilación.

Compilación normal: comandos de `AGENTS.md`. Si CMake reciente exige la
compatibilidad de políticas, agregar `-DCMAKE_POLICY_VERSION_MINIMUM=3.5`.
Para Debug ejecutar solamente la tarea `CMS32L051: Build debug`, que configura
`build-debug-cms32` con `-Og`. El valor por defecto sigue siendo `-Os`.
La configuración `Flash + debug desde main` sí programa el MCU: ejecutarla
únicamente con pedido explícito y montaje de banco seguro.

## Respaldos preservados

`respaldo-migracion-2026-10-21/` conserva los archivos útiles encontrados en
`local-backups/`, `build-arm/` y `build-debug-cms32/`, respetando sus rutas de
origen. Incluye EEPROM históricas, lecturas SWD repetidas, imágenes normal y
Debug, sus ELF y cabeceras de versión. `SHA256SUMS.txt` identifica cada copia.
Son evidencia histórica; el nombre de un archivo no prueba una calibración
correcta ni que esa imagen sea la actualmente flasheada. No restaurarlos de
forma automática. No restaurar v10 cruda después de migrar a v11.

Los archivos generados llamados `Version.h` y los ejecutables ELF sin extensión
están incluidos intencionalmente mediante `git add -f` por las reglas globales
de exclusión. Los builds originales permanecen locales e ignorados.

El backup original del fabricante mencionado en la bitácora no se identificó
entre estos archivos; si está fuera del repositorio, conservarlo por separado,
junto con instaladores o documentos externos que se necesiten.

## Próximo trabajo

Verificar primero que el target CMS compila en la nueva PC. Mantener los límites
de 50 W y 5 A. Continuar la validación ADC y de estados seguros antes de probar
el lazo de potencia. No hubo mediciones ni flasheo durante este traspaso.
