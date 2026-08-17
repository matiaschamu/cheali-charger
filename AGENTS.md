# AGENTS.md - Port del iMAX B6 80W a CMS32L051

## Objetivo y alcance

Este fork adapta cheali-charger al clon iMAX B6 80W con MCU Cmsemicon
CMS32L051 (Cortex-M0+). La prioridad es lograr un port seguro, comprobable e
incremental, modificando primero los archivos específicos del CMS32L051 y el
mínimo indispensable del núcleo compartido.

Repositorio original: <https://github.com/stawel/cheali-charger>

Fork: <https://github.com/matiaschamu/cheali-charger>

## Reglas obligatorias de trabajo

1. Conservar los cambios del usuario. Antes de editar, ejecutar `git status
   --short --branch` y revisar el diff de los archivos que se tocarán. No
   descartar, reformatear ni incluir cambios ajenos a la tarea.
2. Preferir cambios bajo `src/hardware/cms32-CMS32L051/`. Tocar `src/core/`
   sólo cuando sea necesario agregar una interfaz genérica o un hook débil que
   no contenga conocimiento específico del chip.
3. Mantener compilables los otros ports. Un cambio compartido debe tener un
   comportamiento por defecto compatible con AVR y Nuvoton.
4. No confundir "compila" con "funciona en hardware". Usar en notas y commits
   estas etiquetas:
   - `[MANUAL]`: respaldado por manual/datasheet del fabricante.
   - `[ESQUEMA]`: respaldado por los esquemas de esta placa.
   - `[CONTINUIDAD]`: seguido físicamente con multímetro, sin probar la función.
   - `[MEDIDO]`: comprobado físicamente, indicando instrumental y condiciones.
   - `[COMPILA]`: verificado sólo por compilación.
   - `[PENDIENTE]`: todavía no comprobado.
5. No cambiar frecuencia PWM, polaridad, topología, límites de potencia,
   option bytes, secuencia de flasheo o mapa ADC por intuición. Citar el
   manual/esquema o registrar una medición.
6. No flashear el cargador ni ejecutar pruebas que energicen MOSFETs sin pedido
   explícito del usuario. Preparar firmware y procedimiento no implica permiso
   para accionar hardware.
7. Hacer commits pequeños por función: clock, GPIO, ADC, PWM, flash, etc. No
   mezclar pruebas de banco con refactors generales.
8. Actualizar este archivo al terminar cada avance significativo: estado,
   evidencia, riesgos, prueba realizada y siguiente paso.
9. No crear commits hasta que el usuario indique explícitamente qué debe
   incluir cada uno. Usar el autor configurado por el usuario, mensajes breves
   y coherentes con el historial del repositorio; no agregar atribución de IA.

## Seguridad de banco

Este firmware controla un convertidor buck/boost y baterías de litio. Un error
de pin, polaridad o duty puede destruir MOSFETs, pistas, batería o instrumental.

- Arrancar siempre con salida cortada, modo cargador seguro y PWM en 0%.
- Para las primeras pruebas usar fuente con límite de corriente, fusible y
  carga resistiva/electrónica; no usar una LiPo como primera carga.
- Comprobar con osciloscopio frecuencia, duty, amplitud de compuerta, tiempos
  muertos y ausencia de pulsos al iniciar, detener y cambiar de topología.
- Validar ADC y calibración antes de habilitar lazo cerrado.
- Mantener inicialmente los límites conservadores actuales de 50 W y 5 A. El
  nombre comercial "80W" no demuestra que la etapa pueda operar allí de forma
  continua.
- El menú `Options -> buck test` es una herramienta peligrosa de laboratorio.
  Debe permanecer manual, nunca ejecutarse al arranque y usarse sólo con una
  configuración de banco segura.

## Arquitectura relevante

El flujo de armado es:

`CMakeLists.txt` -> target -> `CHEALI_CPU(cms32-CMS32L051)` ->
`cpu/cpu.cmake` -> `CHEALI_GENERIC_CHARGER(50W)` ->
`generic/50W/generic.cmake` -> `src/core/core.cmake`.

Las capas son:

- `src/core/`: algoritmos de carga, menús, calibración, LCD, settings y API
  común. Debe seguir independiente del CMS32L051.
- `src/hardware/cms32-CMS32L051/cpu/`: arranque, clock, SysTick, GPIO, serial,
  memoria y adaptación CMSIS del MCU.
- `src/hardware/cms32-CMS32L051/generic/50W/`: hardware de esta familia de
  cargador: pines, ADC, control SMPS, PWM y prueba buck/boost.
- `src/hardware/cms32-CMS32L051/targets/imaxB6-80W-cms32L051/`: límites,
  selección del target y calibración inicial.
- `src/hardware/cms32-CMS32L051/cpu/CMSIS/`: CMSIS y StdDriver del fabricante.
  Tratarlo como código vendor. Si se modifica, explicar por qué no alcanza un
  wrapper del port y dejar el cambio mínimo y localizado.
- `utils/CMS32L051_flash_tools/`: programación por OpenOCD/ST-Link.
- `docs/imaxB6-80W-Cmsemicon-CMS32L051/`: esquemas y documentación de MCU y
  MOSFETs.

El target de referencia funcional para comparar abstracciones es
`src/hardware/nuvoton-M051/`, especialmente `generic/50W/`, pero no se deben
copiar registros, canales, polaridades ni supuestos de periféricos.

## Estado del repositorio relevado al 2026-08-16

- Rama activa: `buck-boost-pruebas`, commit `971805f6`.
- La rama activa está un commit por delante de `Soporte_CMS32L051`.
- `Soporte_CMS32L051` está 7 commits por delante de
  `origin/Soporte_CMS32L051` (`ef7cc205`). Hay trabajo local todavía no
  publicado en esa rama remota.
- Tras actualizar remotos, `upstream/master` está en `3afe8f4f` y la rama
  activa está 0 commits detrás / 21 delante del upstream.
- Existe `origin/Support-cms32l051` con un intento anterior. No mezclarlo sin
  comparar primero su historial.
- Cambios locales sin commit que deben preservarse:
  - `cpu/CMSIS/StdDriver/src/gpio.c`: al entrar en modo analógico fuerza PM a
    entrada y desactiva open-drain; intenta prevenir lecturas saturadas como la
    sospechada en P17/ANI20, pero todavía no se probó en placa.
  - `generic/50W/outputPWM.cpp`: experimento PWM a 60 kHz, extremos 0/100% como
    GPIO y compensación `TDR = high_ticks - 1`.
- No versionados al hacer este relevamiento:
  - `build-arm/`.
  - `CMS32L051 User Manual_V1.2.3.pdf`.
  - `NCE4435.pdf` y `NCE6050KA.pdf`.
  No agregarlos automáticamente a un commit.

Volver a comprobar todos estos datos al iniciar una sesión; esta sección es una
foto histórica, no reemplaza `git status` ni `git fetch`.

## Banco y hardware confirmados al 2026-08-17

- Está flasheado el último firmware de la rama activa. Pantalla, botones,
  buzzer y navegación de menús funcionan.
- El mensaje observado es `please calibrate`. El core lo muestra después de
  restaurar correctamente la EEPROM a valores por defecto; indica que falta
  realizar la calibración, no un error ADC.
- Los cambios locales de GPIO analógico y PWM sólo fueron compilados; ninguno
  fue probado físicamente.
- Los pines se comprobaron por continuidad. Todavía no se midió ningún canal
  ADC ni se probaron funcionalmente P00, P20, P21 o P15.
- La elección experimental de 60 kHz se tomó de otro cargador, no de una
  medición del firmware original ni de esta placa.
- PCB: `B6-CMS V12`. Los esquemas disponibles coinciden hasta ahora con la
  placa, aunque no fueron dibujados por el usuario.
- Los componentes de potencia listados en el esquema fueron confirmados.
- Hay osciloscopio sin sonda diferencial, multímetro, fuente regulable con
  límite de corriente, carga electrónica/resistiva y batería simulada. Las
  masas se comparten; conectar la pinza de masa del osciloscopio únicamente al
  GND común confirmado y no a nodos flotantes o conmutados.
- ST-Link y OpenOCD están conectados. Existe backup del firmware original y
  datos de recuperación.
- No se comprobó todavía persistencia de EEPROM ni el tamaño real de borrado
  flash.
- UART no se necesita y el conector/señales no son físicamente accesibles en
  este equipo. No priorizar su implementación.
- Objetivo de largo plazo: funcionamiento completo y luego validación gradual
  hasta 80 W. Orden inmediato: ADC, prueba manual buck/boost, carga real a baja
  corriente y por último uso normal.
- El menú `buck test` se conserva durante el desarrollo y se retirará antes de
  aproximar el port a la presentación del proyecto original.
- No se creará un target ADC separado. Las pruebas se integrarán de forma
  temporal y segura en el firmware normal, bajo control explícito del usuario.

## Matriz de avance del port

### Implementado y respaldado

- `[COMPILA]` Target `imaxB6-80W-cms32L051` integrado a CMake.
- `[MANUAL][COMPILA]` Clock interno HOCO a 48 MHz; option bytes colocados en
  `0xC0..0xC2`. SysTick genera el tick de 500 us.
- `[ESQUEMA]` Mapeo de LCD, botones, buzzer, UART, entradas analógicas,
  balanceadores y control de potencia en `imaxB6-pins.h`.
- `[MEDIDO]` Pantalla, botones, buzzer y navegación de menús funcionan con el
  último firmware flasheado.
- `[MANUAL][COMPILA]` Tabla pin -> canal ANI implementada en `cpu/IO.cpp`. El
  manual confirma, entre otros, P17=ANI20, P62=ANI27, P75=ANI34 y P136=ANI36.
- `[COMPILA]` Adquisición ADC real por interrupción y recorrido de entradas en
  `AnalogInputsADC.cpp`.
- `[ESQUEMA][CONTINUIDAD]` El trazado físico coincide hasta ahora para
  Vout+/Vout-, entradas analógicas, UART, balanceadores y señales de potencia.
  Esto no valida canales ADC, polaridades ni comportamiento funcional.
- `[MANUAL][COMPILA]` Driver PWM por TM41/TO11. El diagrama del Timer4 confirma
  que el período efectivo usa `TDR+1`, base de la compensación actual.
- `[COMPILA]` EEPROM emulada en el último 1 KiB de flash (`0xFC00..0xFFFF`) y
  cargada al arrancar mediante un hook débil en el core.
- `[COMPILA]` Flasheador OpenOCD que preserva la sombra EEPROM al actualizar el
  firmware.
- `[MEDIDO]` Backup y verificación EEPROM probados por OpenOCD: lectura de 1024
  bytes desde `0xFC00`, archivo con firma `chli` y comparación binaria correcta.
- `[PENDIENTE]` `cms32_flash_safe` y `cms32_eeprom_restore` están implementados
  en el working tree, pero todavía no se probó ninguna operación de borrado o
  programación con estos procedimientos.
- `[COMPILA]` Menú manual para ejercitar buck/boost.

### Pendiente o de riesgo alto

- `[PENDIENTE]` Validar el PWM local de 60 kHz en osciloscopio: frecuencia,
  duty mínimo/máximo, transición 0%/PWM/100%, glitches y temperatura. No dar
  por cierta la frecuencia elegida ni la afirmación de SOA del comentario sin
  medición en esta placa.
- `[PENDIENTE]` Medir por primera vez todos los canales ADC con la etapa de
  potencia deshabilitada y comparar valor crudo, valor mostrado y multímetro.
- `[PENDIENTE]` Probar funcionalmente y confirmar polaridad segura de P00
  (corte), P20 (carga/descarga), P21 (buck/boost) y P15 (PWM). Hoy sólo están
  seguidos por esquema/continuidad.
- `[PENDIENTE]` Integrar y ajustar el lazo cerrado SMPS sólo después de validar
  ADC, escalas, polaridades y PWM a baja potencia.
- `[PENDIENTE]` Validar descarga, balanceo de las seis celdas y corrección de
  Vb0 durante descarga.
- `[PENDIENTE]` La calibración por defecto fue heredada del port de referencia;
  no es una calibración segura para esta placa. Cada cargador debe calibrarse.
- `[PENDIENTE]` `TxHardSerial.cpp` y `TxSoftSerial.cpp` son stubs: el firmware
  compila, pero no transmite logs UART. No es prioritario porque no hay acceso
  físico al conector.
- `[PENDIENTE]` Los pines placeholder `SMPS_DISABLE_PIN`,
  `DISCHARGE_VALUE_PIN` y `SMPS_VALUE_BOOST_PIN` no representan señales usadas
  por esta placa. No activarlos accidentalmente desde código genérico.
- `[PENDIENTE]` Evaluar desgaste, tiempo de bloqueo y tolerancia a corte de
  energía de la EEPROM emulada: hoy cada escritura resincroniza toda la
  estructura de aproximadamente 992 bytes.
- `[PENDIENTE]` Resolver/documentar la discrepancia de borrado flash: el
  datasheet V1.9.6 declara páginas de 512 bytes, pero la herramienta y la
  emulación usan sectores de 256 bytes. El usuario no realizó la prueba que
  justificaría 256 bytes; tratar ese valor como no verificado hasta diseñar
  una prueba no destructiva y documentada.
- `[PENDIENTE]` Revisar la habilitación
  `ENABLE_EXT_TEMP_AND_UART_COMMON_OUTPUT`: UART todavía es stub, por lo que la
  opción visible no implica funcionalidad real.

## Compilación y verificación

Configuración habitual en Windows:

```powershell
cmake -S . -B build-arm -G "MinGW Makefiles" `
  -DCMAKE_TOOLCHAIN_FILE=arm-toolchain.cmake `
  -Denable-short-names=ON
```

Compilar sólo el target CMS32L051:

```powershell
cmake --build build-arm `
  --target imaxB6-80W-cms32L051_cms32-CMS32L051 `
  --parallel 4
```

Artefactos esperados:

`build-arm/src/hardware/cms32-CMS32L051/targets/imaxB6-80W-cms32L051/`

- `imaxB6-80W-cms32L051_cms32-CMS32L051`
- `imaxB6-80W-cms32L051_cms32-CMS32L051.hex`
- `imaxB6-80W-cms32L051_cms32-CMS32L051.bin`

En el relevamiento, el target CMS compiló y produjo 31,396 bytes de texto, 32
bytes de data y 3,240 bytes de BSS. La imagen `.bin` midió 31,428 bytes. La
flash queda dividida en 63 KiB para programa y 1 KiB para EEPROM; SRAM total:
8 KiB.

El target `.size` puede fallar en Windows porque `arm-none-eabi-size` no está en
`PATH`, aunque CMake haya encontrado el compilador por nombre corto. Usar el
ejecutable de la misma toolchain o corregir CMake en una tarea separada.

La compilación global `cmake --build build-arm` actualmente falla en targets
Nuvoton heredados: archivos C como `_syscalls.c` reciben cabeceras C++ y fallan
con `unknown type name 'namespace'`. No atribuir ese fallo al port CMS ni
arreglar los ports Nuvoton salvo pedido explícito. La validación mínima de este
trabajo es el target CMS aislado más `git diff --check`.

## Flasheo

Leer primero `utils/CMS32L051_flash_tools/README.md`. El flujo usa ST-Link,
OpenOCD y acceso directo al FMC porque OpenOCD no tiene driver nativo para este
MCU.

La rutina `cms32_flash` borra sólo `0x0000..0xFBFF` y debe preservar
`0xFC00..0xFFFF`. No reemplazarla por chip erase durante actualizaciones si se
quiere conservar calibración y settings. La programación byte a byte puede
tardar varios minutos.

Durante el desarrollo preferir `cms32_flash_safe` y conservar un backup EEPROM
distinto por cada estado estable. El backup sólo debe restaurarse sobre una
versión compatible del layout EEPROM (`e10.3.12` actualmente).

Antes de flashear, verificar:

1. Que el `.bin` pertenece al target CMS correcto.
2. Que los option bytes aparecen en las direcciones esperadas.
3. Que el tamaño no invade `0xFC00`.
4. Que existe backup del firmware/datos si la prueba puede ser destructiva.
5. Que la fuente del cargador está limitada en corriente para el primer boot.

## Estrategia incremental recomendada

1. Caracterizar el aviso/error de calibración y medir todos los ADC con la
   etapa de potencia deshabilitada, incluyendo el cambio analógico local.
2. Confirmar estados seguros de todos los pines durante reset, arranque, menú y
   apagado de PWM.
3. Consolidar el cambio GPIO sólo después de comparar las lecturas ADC con y
   sin él.
4. Probar el PWM de 60 kHz open-loop con osciloscopio y carga segura, primero
   buck y luego boost, en duty y corriente bajos.
5. Habilitar lazo cerrado a baja potencia y ajustar límites/PID con capturas de
   corriente y tensión.
6. Validar descarga y luego balanceadores, una celda por vez.
7. Implementar UART para obtener telemetría reproducible.
8. Mejorar robustez/desgaste de EEPROM.
9. Sólo después realizar calibración completa y pruebas térmicas crecientes.
   Elevar de 50 W hacia 80 W únicamente si las mediciones lo justifican.

## Formato de bitácora

Agregar entradas nuevas arriba de las anteriores. Una entrada útil incluye:

```text
Fecha / commit o working tree:
Objetivo:
Archivos modificados:
Montaje: fuente, límite de corriente, carga/batería, instrumental.
Resultado: [MANUAL] [ESQUEMA] [MEDIDO] [COMPILA] [PENDIENTE]
Mediciones: pines, frecuencia, duty, tensión, corriente, temperatura.
Riesgos o anomalías:
Próximo paso mínimo:
```

### 2026-08-17 - Punto de partida para pruebas de ADC y PWM

- `[COMPILA]` El target aislado
  `imaxB6-80W-cms32L051_cms32-CMS32L051` compiló correctamente con la
  toolchain ARM instalada. Los avisos de `_close`, `_lseek`, `_read` y `_write`
  son stubs heredados de newlib y no impidieron generar el firmware.
- Se corrigieron comentarios para no presentar como medición el modo analógico
  de GPIO ni la frecuencia PWM experimental de 60 kHz. Ambos cambios siguen
  pendientes de validación física.
- Las herramientas de backup y verificación EEPROM están listas; sólo se
  probaron las operaciones no destructivas. Restauración y flasheo seguro aún
  no se ejecutaron.
- No se flasheó memoria ni se energizaron las salidas de potencia durante esta
  verificación.
- Próximo paso mínimo: medir los canales ADC con la etapa de potencia
  deshabilitada y conservar el backup EEPROM ya validado.

### 2026-08-17 - Conexión SWD original confirmada tras corrección eléctrica

- El usuario identificó y corrigió la causa eléctrica de los fallos anteriores.
- Se restauró `target_CMS32L051_win.cfg` a `interface/stlink.cfg`; no se mantiene
  ningún cambio al transporte HLA.
- `[MEDIDO]` OpenOCD conectó por SWD dapdirect con el ST-Link V2J46S7, tensión
  de target de 3,268 V, DPIDR `0x0bc11477` y detección del Cortex-M0+ r0p1.
- `[MEDIDO]` Con esa configuración se volvió a comparar la EEPROM completa
  (`0xFC00..0xFFFF`) con el backup existente y resultó idéntica.
- No se borró ni programó memoria. El MCU se reanudó con `reset run`.
- Los errores USB registrados en los intentos anteriores quedan como historial
  del diagnóstico, pero no demuestran una incompatibilidad de dapdirect.

### 2026-08-17 - Backup EEPROM validado

- El primer acceso exitoso se realizó temporalmente mediante HLA durante el
  diagnóstico. Más tarde se confirmó dapdirect tras corregir el problema
  eléctrico.
- Backup creado en
  `local-backups/cms32l051/eeprom-2026-08-17-before-adc.bin`, tamaño 1024 bytes.
- SHA-256:
  `0420E37F5266CD02128B508ABF50BAC8A41EC227D1897FC28B69607EE731BE54`.
- Los primeros bytes son `63 68 6C 69` (`chli`), firma esperada de la EEPROM de
  cheali-charger. `cms32_eeprom_verify` confirmó igualdad con la flash; el CRC
  auxiliar de OpenOCD agotó tiempo y la herramienta completó la comparación
  binaria correctamente.
- `local-backups/` quedó ignorado por Git.
- No se borró ni programó memoria. El CMS32L051 fue reanudado con `reset run`.

### 2026-08-17 - Segundo intento de backup EEPROM

- El ST-Link ya es detectado por OpenOCD, pero `init` falla con
  `claim interface failed` y transferencia USB `error -5`.
- No se encontró otro proceso OpenOCD/STM32/ST-Link ocupando el adaptador. El
  diagnóstico apunta al driver de Windows; revisar/asignar WinUSB con Zadig al
  dispositivo ST-Link correcto.
- No se creó backup y no se leyó, borró ni programó memoria del CMS32L051.

### 2026-08-17 - Primer intento de backup EEPROM

- OpenOCD localizado en
  `C:/Users/Matias/Downloads/openocd/xpack-openocd-0.12.0-7/bin/openocd.exe`.
- La versión xPack 0.12.0 cargó correctamente la configuración y los nuevos
  procedimientos TCL, pero `init` falló en `stlink_usb_usb_open(): open failed`.
- Windows no enumeró un dispositivo conectado con VID ST-Link `0x0483`. Revisar
  conexión, alimentación, aplicación que pueda ocuparlo y driver WinUSB.
- No se creó backup, no se leyó EEPROM y no se borró/programó ninguna región.

### 2026-08-17 - Preservación de EEPROM durante el desarrollo

- Se confirmó que `please calibrate` es el mensaje normal posterior a una
  restauración exitosa de valores por defecto.
- El linker reserva `0xFC00..0xFFFF` como `NOLOAD`, por lo que el `.bin` no
  incluye la EEPROM. El borrado normal de firmware termina en `0xFBFF`.
- Se agregó, sin commit, `cms32_flash_safe`, backup/verificación/restauración
  explícita de EEPROM y un límite de tamaño para impedir que una imagen de
  firmware invada la sombra.
- Backup y verificación ya fueron validados posteriormente. Falta probar
  persistencia tras un flasheo seguro y, sólo si fuera necesario, restauración.

### 2026-08-17 - Estado del hardware informado por el usuario

- Último firmware flasheado; LCD, botones, buzzer y menús funcionan. El mensaje
  observado es `please calibrate` después de restaurar valores por defecto.
- Los mapeos se comprobaron por continuidad, no mediante mediciones ADC ni
  pruebas de las señales de potencia.
- Los cambios locales de GPIO y PWM sólo compilan. La frecuencia de 60 kHz se
  tomó como referencia de otro cargador y aún debe justificarse en esta placa.
- Hay instrumental completo, carga electrónica/batería simulada, ST-Link,
  OpenOCD y backup recuperable. No hay sonda diferencial.
- Se acordó comenzar por ADC con potencia deshabilitada. No hacer commits hasta
  que el usuario indique contenido y momento.

### 2026-08-16 - Relevamiento inicial para agentes

- Se estudió la estructura completa del port, sus 21 commits sobre upstream,
  el estado de ramas, los diffs locales, esquemas, datasheet y capítulos
  relevantes del manual de usuario del CMS32L051.
- El target CMS aislado compila. La compilación ARM completa falla en ports
  Nuvoton heredados por una incompatibilidad C/C++ ajena a esta adaptación.
- Se identificaron como próximos pasos inmediatos la prueba física del fix
  analógico de P17 y la validación segura del nuevo PWM de 60 kHz.
