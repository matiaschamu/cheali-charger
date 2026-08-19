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
- La EEPROM ya se preservó y verificó durante un flasheo completo. Sigue
  pendiente resolver documentalmente la discrepancia del tamaño de sector de
  borrado (256 bytes empíricos frente a 512 bytes del datasheet).
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
- `[MEDIDO]` Flasheador OpenOCD probado con `cms32_flash_safe`: crea backup,
  borra/programa sólo firmware, verifica firmware y confirma que la sombra
  EEPROM no cambió antes de reiniciar.
- `[MEDIDO]` Backup y verificación EEPROM probados por OpenOCD: lectura de 1024
  bytes desde `0xFC00`, archivo con firma `chli` y comparación binaria correcta.
- `[PENDIENTE]` `cms32_eeprom_restore` está implementado, pero todavía no se
  probó su operación destructiva de borrado y reprogramación de la sombra.
- `[COMPILA]` Menú manual para ejercitar buck/boost.

### Pendiente o de riesgo alto

- `[PENDIENTE]` Validar el PWM local de 30 kHz en osciloscopio: frecuencia,
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
4. Probar el PWM de 30 kHz open-loop con osciloscopio y carga segura, primero
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

### 2026-08-19 - Suite CMS de pruebas de potencia

- Archivos modificados: `generic/50W/BuckTest.cpp` y `BuckTest.h`. No se
  agregaron cambios en `src/core/`; se reutiliza el hook condicional existente
  `Options -> buck test`.
- `[COMPILA]` La entrada abre seis pruebas específicas del CMS: PWM buck
  manual, mando combinado buck/boost, ADC crudo con potencia activa,
  carga/descarga mediante P20, corte P00 y balanceadores individuales.
- `[COMPILA]` El mando combinado recorre `B000..B100` y luego `G001..G050`.
  El auto-repetido se detiene exactamente en B100/G001 para no saltar el punto
  de transición. Al cruzarlo conserva P00 conectado y ejecuta P15 bajo, cambio
  de P21 y reanudación con el nuevo duty, sin demora intencional.
- `[COMPILA]` Boost queda limitado al 50 %, igual que
  `MAX_PID_MV_FACTOR=1.5`. La prueba de descarga queda limitada temporalmente
  al 20 % hasta medir U9 e `Idischarge`.
- `[COMPILA]` Todas las entradas y salidas de prueba convergen a P15 bajo, P21
  bajo, P20 alto, P00 alto y balanceadores apagados. Ninguna prueba se ejecuta
  al arrancar y cada salida debe armarse con START desde duty cero.
- `[COMPILA]` Target CMS aislado: imagen de 33.544 bytes, option bytes
  `EE 36 E0`, SHA-256
  `D569FCC286AEAE9C4B63BF302A32C5C08C2D3275A8696105E8D639532055491D`.
- `[PENDIENTE]` Flashear con autorización y validar primero la navegación y
  estados de reposo; después capturar B100->G001 y G001->B100 midiendo P15,
  P21 y las compuertas de Q9/U8.

### 2026-08-19 - Actualización continua de duty TM41 preparada

- Archivo modificado: `generic/50W/outputPWM.cpp`.
- `[MANUAL]` El manual CMS32L051 V1.2.3 indica que `TDRmn` puede reescribirse
  en cualquier momento y que generar nuevamente el trigger `TSmn` durante la
  operación reinicializa el contador.
- `[COMPILA]` El arranque de un duty intermedio todavía configura TM41 y
  dispara una sola vez `TS1`. Los cambios posteriores entre 1 y 99 % escriben
  únicamente `TDR11`, sin detener ni reiniciar los canales maestro/esclavo.
- A pedido del usuario, el período se restauró a 30 kHz: reloj de 48 MHz,
  `TDR10=1599` y 1600 ticks por período. La resolución queda en 0,0625 %.
- Los extremos 0 y 100 % continúan deteniendo TM41 y fuerzan P15 como GPIO
  bajo/alto respectivamente. Volver desde un extremo requiere un nuevo
  arranque y no forma parte de la prueba de continuidad entre duties PWM.
- `[MEDIDO]` Se acepta como resuelto que el exceso de ancho visto anteriormente
  en la señal de compuerta provenía del driver MOSFET y no del PWM generado por
  TM41.
- `[COMPILA][MEDIDO]` Imagen de 31.880 bytes, option bytes `EE 36 E0` y
  SHA-256 `5D850DEB1741C53F8D43F5D8E4DA73AD593E1BB2A6F5CCB4B3A3E5A3BF32474D`.
  Se flasheó mediante `cms32_flash_safe` con ST-Link a 3,253 V; OpenOCD
  verificó el firmware, confirmó `EEPROM backup matches flash` y reinició el
  MCU.
- Backup `eeprom-2026-08-19-before-continuous-pwm-30khz.bin`, 1.024 bytes,
  SHA-256 `0420E37F5266CD02128B508ABF50BAC8A41EC227D1897FC28B69607EE731BE54`.
- `[MEDIDO]` Con osciloscopio sobre P15 a 30 kHz, el usuario observó pulsos
  correctos y continuidad entre cambios de duty intermedios: no aparecieron
  saltos ni pulsos anómalos. Queda validada la actualización de `TDR11` sin
  reiniciar TM41.
- `[COMPILA][PENDIENTE]` La suite nueva ya permite capturar la transición de
  producción con P00 conectado, pero todavía no fue flasheada ni medida.

### 2026-08-19 - PID adaptado al PWM único y selectores CMS

- Archivos modificados: `generic/50W/SMPS_PID.cpp` y `imaxB6.cpp`.
- `[COMPILA]` El control normal usa exclusivamente TM41/TO11/P15 para buck,
  boost y descarga. El valor `pin` pasado a `outputPWM` es ahora P15 y no los
  alias heredados de canales Nuvoton.
- `[COMPILA]` Cada rama de `setPID_MV()` escribe explícitamente P21: bajo para
  buck y alto para boost. Cuando cambia la topología, primero detiene P15,
  luego cambia P21 y finalmente aplica el nuevo duty. Esto corrige el estado
  boost pegado al regresar a buck.
- `[COMPILA]` P20 queda alto durante carga y reposo y bajo sólo durante
  descarga. Apagar carga o descarga fuerza P15 bajo y P21 bajo. P00 continúa
  controlado únicamente por `setBatteryOutput()` como corte general.
- `[COMPILA]` Se retiraron tanto las escrituras de ejecución como la
  configuración `OUTPUT` de los placeholders P23 (`SMPS_DISABLE_PIN`), P24
  (`DISCHARGE_VALUE_PIN`) y P26 (`SMPS_VALUE_BOOST_PIN`). Las definiciones se
  conservan sólo para compatibilidad de cabeceras; ningún `.cpp` CMS las usa.
- Target CMS aislado compilado: `.bin` de 31.872 bytes, option bytes
  `EE 36 E0`, SHA-256
  `A8F3D95A9184C087333BEE42EBB43DCFFF62CBDE1A68CFA0B759A1C698B4736F`.
  `git diff --check` sin errores.
- `[MEDIDO]` Se flasheó mediante `cms32_flash_safe` con ST-Link a 3,242 V.
  OpenOCD verificó firmware, confirmó `EEPROM backup matches flash` y reinició
  el MCU. Backup `eeprom-2026-08-19-before-f2c3c8ce.bin` de 1.024 bytes,
  SHA-256 `0420E37F5266CD02128B508ABF50BAC8A41EC227D1897FC28B69607EE731BE54`.
- `[PENDIENTE]` Antes del lazo cerrado se debe comprobar en hardware la
  secuencia P15/P21 al cruzar buck↔boost y resolver la actualización de duty
  que todavía reinicia TM41 en cada llamada.

### 2026-08-18 - Interpretación de P15, P20, P21 y transistores asociados

- `[ESQUEMA]` P15 es la modulación común. Una rama por R13 llega al driver
  complementario Q3/Q4 del MOSFET NCE4435 (Q9, conmutador buck); otra rama por
  R11 alimenta la lógica de diodos asociada a P21 y la entrada de Q2.
- `[ESQUEMA][MEDIDO]` P21 es el selector buck/boost. En buck permanece bajo y
  bloquea el driver Q5/Q6 de U8 (NCE6050KA); en boost se lleva alto para que
  esa rama pueda conmutar U8. La forma exacta en que mantiene Q9 durante boost
  debe confirmarse midiendo las compuertas de Q9 y U8 simultáneamente.
- `[ESQUEMA]` Q2 es un MOSFET pequeño de pre-driver, no el transistor de
  potencia. Recibe la rama PWM de P15 y gobierna el nodo de entrada del par
  complementario V22/V39, cuya salida maneja la compuerta de U9 (NCE0110AK).
- `[ESQUEMA]` P20, mediante R10 y Q1, fuerza/clampa ese nodo de pre-driver. Con
  P20 alto Q1 conduce y anula la rama de U9, coherente con modo carga; con P20
  bajo Q1 se libera, coherente con habilitar la ruta de descarga. La polaridad
  y el PWM efectivo sobre U9 durante descarga siguen `[PENDIENTE]` de medición.
- `[ESQUEMA]` Q10 no pertenece a la conversión de potencia. Su base viene de
  P27 por R29=510 ohm, su rama superior va a 5P por R33=470 ohm y la inferior a
  masa. P27 también es D4 del LCD. Tal como está dibujado, Q10 sólo introduce
  una carga conmutada sobre esa línea y no entrega una salida a otro bloque;
  su finalidad exacta o una posible inexactitud del esquema inverso quedan
  `[PENDIENTE]` y no deben mezclarse con P15/P20/P21.
- Próximo paso mínimo: con carga limitada, observar simultáneamente P15 y las
  compuertas de Q9/U8 para buck y boost. Dejar U9/descarga para una prueba
  separada después de confirmar el estado impuesto por P20.

### 2026-08-18 - Auditoría de control P15/P21/P20/P00 en firmware

- `[COMPILA][MEDIDO]` `outputPWM.cpp` posee un solo canal real: cualquier
  llamada a `setPWM()` o `disablePWM()`, sin importar el argumento `pin`, actúa
  sobre TM41/TO11/P15. Cero y deshabilitado fuerzan P15 bajo; escala completa
  fuerza P15 alto; valores intermedios generan PWM de 60 kHz.
- `[COMPILA][MEDIDO]` P21 se usa como GPIO selector: bajo=buck, alto=boost. P20
  se usa como GPIO de modo: alto=carga, bajo=descarga. P00 es corte de salida
  activo alto: alto=salida desconectada, bajo=salida conectada.
- En arranque y en `BuckTest` la secuencia queda segura: P00 alto, P15 bajo,
  P21 bajo y P20 alto al estar apagado. Al iniciar la prueba, P00 baja, P20 se
  mantiene alto, P21 selecciona el modo y P15 entrega PWM.
- `[PENDIENTE]` El `SMPS_PID.cpp` del CMS es idéntico al de Nuvoton M051 y no
  fue adaptado a la arquitectura de PWM único. En Nuvoton/AVR los argumentos
  buck y boost seleccionan canales físicos distintos; en CMS ambos terminan en
  P15 y la selección depende de P21.
- `[PENDIENTE][RIESGO]` En el PID CMS, entrar a boost ejecuta P21=alto, pero la
  rama posterior de buck sólo apaga el placeholder P26 y no devuelve P21 a
  bajo. Por ello una transición boost->buck puede dejar seleccionada la
  topología boost mientras P15 recibe el duty calculado como buck.
- `[PENDIENTE]` El código normal todavía escribe los placeholders P23
  (`SMPS_DISABLE_PIN`), P24 (`DISCHARGE_VALUE_PIN`) y P26
  (`SMPS_VALUE_BOOST_PIN`). El argumento ignorado de `outputPWM` hace que P24
  module realmente P15, pero las escrituras GPIO a P23/P26 no representan el
  hardware CMS y deben eliminarse de la ruta específica.
- Comparación: AVR usa dos salidas Timer1 (buck en pin 13 y boost/descarga en
  pin 14); Nuvoton M051 usa PWM P2.6 para buck y P2.1 compartido para
  boost/descarga. P00 y P20 tienen equivalentes funcionales en esos ports
  (`OUTPUT_DISABLE` y `DISCHARGE_DISABLE` activos altos para deshabilitar),
  pero el selector dedicado P21 y el PWM común P15 son particulares de esta
  placa CMS.
- Próximo cambio mínimo propuesto: adaptar sólo el `SMPS_PID.cpp` CMS para
  usar explícitamente `setTopology(false/true)`, P15 como único PWM y P20 como
  selector carga/descarga, sin escribir P23/P24/P26 como GPIO reales.

### 2026-08-17 - Diagnóstico TM41 corregido, compilado y flasheado

- Archivos modificados: `generic/50W/outputPWM.cpp`, `BuckTest.cpp` y
  `BuckTest.h`.
- `[MEDIDO]` Con el firmware de diagnóstico, la pantalla confirmó
  `TDR10=799` y TDR11 coherente con el duty pedido (aproximadamente 7, 15, 40
  y 80 para 1, 2, 5 y 10 %). En P15 se midieron respectivamente 2,056; 2,018;
  2,91 y 3,86 us. Los anchos teóricos por registro son 0,146; 0,313; 0,833 y
  1,667 us: persiste un exceso cercano a 2 us fuera del cálculo de duty.
- `[MEDIDO][ESQUEMA]` El menú sólo reinicia TM41 al pulsar INC/DEC, no en cada
  iteración. P15/TO11 está cargado en placa por R13=510 ohm y la etapa bipolar
  Q5/Q6 que maneja la compuerta de U8 (NCE6050KA); por lo tanto la medida en el
  nodo y sus flancos deben caracterizarse antes de atribuir el offset al silicio.
- `[PENDIENTE]` Medir en P15 Vmin, Vmax, tiempo de subida, tiempo de bajada y
  ancho a un umbral fijo del 50 %. Si la forma es limpia, preparar una imagen
  diagnóstica que permita `TDR11=0` manteniendo TM41 activo: distinguirá un
  pulso mínimo del periférico de una deformación eléctrica.
- `[COMPILA][MEDIDO]` A pedido del usuario se preparó una variante temporal
  que enruta TO11 a P00 y fuerza P15 a nivel bajo. P00 deja de cumplir su rol
  normal de corte de batería; esta imagen sólo sirve para comparar el ancho de
  pulso y no debe utilizarse para carga ni operación normal.
- La variante P00 produjo un `.bin` de 31.848 bytes, option bytes `EE 36 E0` y
  SHA-256 `1B38E302D77D60F715E1425378F969DDDAB9B258DF76A6AD4954A142AE351B6E`.
  Se cargó con `cms32_flash_safe`, ST-Link a 3,268 V; OpenOCD verificó el
  firmware, confirmó `EEPROM backup matches flash` y reinició el MCU. Backup
  de 1.024 bytes `eeprom-2026-08-17-before-p00-pwm-diagnostic.bin`, SHA-256
  `0420E37F5266CD02128B508ABF50BAC8A41EC227D1897FC28B69607EE731BE54`.
- `[MEDIDO]` Con ST-Link desconectado, P00 midió 5 V con PWM apagado; 1 % =
  142 ns, 2 % = 310 ns, 5 % = 830 ns, 10 % = 1,66 us, 99 % = 16,43 us y
  100 % = 5 V constante. Los valores coinciden con `TDR11/48 MHz` dentro de la
  resolución de medición. Los estados apagado/100 % altos son intencionales en
  esta imagen para mantener activo el corte normalmente gobernado por P00.
- `[MEDIDO]` Queda descartado un offset de aproximadamente 2 us en TM41 y no
  se encontró la supuesta errata del timer. El exceso observado anteriormente
  es específico de P15 o de la etapa R13/Q5/Q6/U8 conectada a ese pin.
- `[PENDIENTE]` Restaurar TO11 a P15 antes de cualquier prueba del convertidor.
  Luego comparar simultáneamente el pin del MCU, ambos lados de R13 y la
  compuerta de U8 para localizar dónde aparece el alargamiento.
- `[COMPILA][MEDIDO]` Tras validar TM41 por P00 se restauró el ruteo normal TO11/P15,
  conservando TDR11 sin compensación, escrituras directas de TS1/TT1 y el
  reinicio sincronizado usado por el menú manual de diagnóstico.
- La imagen P15 restaurada mide 31.852 bytes, contiene option bytes `EE 36 E0`
  y tiene SHA-256
  `DCC31422A08C6FC800A966717571BF855C78DA009EB5F0CCA499D9329C1DF407`.
  Se cargó con ST-Link a 3,251 V mediante `cms32_flash_safe`; OpenOCD verificó
  firmware y EEPROM antes de reiniciar. El backup
  `eeprom-2026-08-17-before-p15-restored.bin` mide 1.024 bytes y conserva el
  SHA-256 `0420E37F5266CD02128B508ABF50BAC8A41EC227D1897FC28B69607EE731BE54`.
- `[MANUAL][COMPILA]` El port configura TM41 directamente mientras está
  detenido, usa `TDR11=high_ticks`, escribe `TS1/TT1` directamente y arranca
  maestro/esclavo juntos. Durante este bring-up cada cambio manual reinicia el
  timer para evitar una actualización asíncrona de TDR11.
- El menú muestra los registros efectivos: primera línea `buck P nnnn` o
  `boost P nnnn` para TDR10; segunda línea `ON d nnn T nnnn` para duty pedido y
  TDR11. Las líneas tienen 16 caracteres completos para evitar residuos.
- `[PENDIENTE]` La parada/reconfiguración en cada cambio es apropiada para el
  diagnóstico manual, pero no para el lazo cerrado. Antes de integrar el PID se
  debe implementar una actualización TDR11 sincronizada con el evento maestro.
- `[COMPILA]` Target aislado: 31.808 bytes de texto, 32 de data y 3.244 de BSS;
  `.bin` de 31.840 bytes, option bytes `EE 36 E0`, SHA-256
  `B04477732E1E88087949C6840AFAEB027BF25AF4D748E194719898CBA4771C0F`.
- `[MEDIDO]` Firmware cargado mediante `cms32_flash_safe` con ST-Link a 3,268 V.
  OpenOCD verificó firmware y confirmó `EEPROM backup matches flash` antes del
  reset. Backup final de 1024 bytes con SHA-256
  `0420E37F5266CD02128B508ABF50BAC8A41EC227D1897FC28B69607EE731BE54`.
- Próximo paso mínimo: desconectar ST-Link, repetir buck a 1, 2, 5 y 10 % y
  registrar TDR10/TDR11 visibles junto al ancho de P15.

### 2026-08-17 - Primera prueba buck: duty real no coincide con el solicitado

- Montaje: entrada a 12 V, límite de fuente de 600 mA, prueba manual buck con
  carga de banco y medición de salida/PWM; pack y ST-Link desconectados.
- `[MEDIDO]` Duty solicitado 0 %: salida 0 V y consumo de entrada 70 mA.
- `[MEDIDO]` Duty solicitado 5 %: osciloscopio indicó 17,4 %, salida 6,4 V y
  consumo de entrada 140 mA.
- `[MEDIDO]` Duty solicitado 10 %: osciloscopio indicó 23,1 %, salida 8,05 V y
  consumo de entrada 150 mA.
- `[MEDIDO]` Anchos de pulso informados: 1 % -> 2,0450 us; 2 % -> 1,96 us;
  5 % -> 2,89 us; 10 % -> 3,84 us. Entre 5 % y 10 % el pulso aumenta 0,95 us,
  pero 1 % y 2 % no son monótonos.
- Los datos son compatibles con un componente de ancho cercano a 2 us más una
  parte variable, o con una medición posterior al MCU. No corregir la escala
  del timer hasta conocer la frecuencia, amplitud y punto exacto de sonda.
- `[MANUAL]` La sección 5.9.3 del manual define el duty como
  `TDR_slave / (TDR_master + 1)`. Con período 800, el código actual debería
  entregar aproximadamente 4,875 % y 9,875 %; la compensación de un tick no
  explica la diferencia medida.
- `[MANUAL]` La sección específica 5.9.2 declara `TDR_slave=0` como 0 % y exige
  que las actualizaciones durante marcha se hagan inmediatamente después del
  evento del maestro. El port resta una cuenta y actualiza `TDR11` de forma
  asíncrona; ambos comportamientos deben corregirse.
- `[COMPILA]` La desensamblación del firmware flasheado confirma que el binario
  calcula y escribe TDR11=6, 14, 39 y 79 para 1, 2, 5 y 10 %, respectivamente,
  con TDR10=799. No hay un offset de 100 cuentas introducido por la aritmética
  C++ o por el compilador.
- La documentación oficial disponible y el ejemplo de Timer4 publicado con el
  driver del fabricante no registran una errata de aproximadamente 2 us. Sí se
  encontró que las funciones vendor de inicio/parada hacen lectura-modificación-
  escritura (`TS1 |=` / `TT1 |=`) sobre registros de disparo, aunque el manual
  prescribe escribir directamente los bits de comando.
- `[PENDIENTE]` No probar boost aún. Confirmar que la sonda estuvo directamente
  en P15 y registrar nivel bajo, nivel alto y tiempos de subida/bajada. Agregar
  temporalmente lectura visible de TDR10/TDR11 al menú, reemplazar el acceso RMW
  a TS1/TT1 por escritura directa y seguir la secuencia de parada/configuración/
  arranque del manual antes de repetir la medición.

### 2026-08-17 - Los siete canales de balance responden con pack 6S

- Verificación previa del pack 6S, referida a su negativo: 0,000; 3,930;
  8,000; 11,920; 15,870; 19,490 y 23,610 V. Las celdas calculadas son 3,930;
  4,070; 3,920; 3,950; 3,620 y 4,120 V. El pack está desbalanceado y se usó
  sólo para sensado, sin carga ni balanceo.
- `[MEDIDO]` Con sólo el conector de balance: Vb0 0,000 V -> 0; Vb1 3,930 V
  -> 43062; Vb2 8,000 V -> 49524; Vb3 11,920 V -> 51405; Vb4 15,870 V ->
  49246; Vb5 19,490 V -> 49601; Vb6 23,610 V -> 50205.
- Los siete canales responden, están dentro de rango y ninguno satura. La
  reducción de cuentas entre canales consecutivos es esperable porque poseen
  divisores diferentes; las cuentas no representan directamente la tensión.
- Frente al ensayo 4S, Vb1..Vb4 cambiaron sólo 160, 84, 58 y 40 cuentas
  respectivamente (aproximadamente 0,37 %, 0,17 %, 0,11 % y 0,08 %), una
  repetibilidad funcional razonable pese al ruido ya identificado.
- `[MEDIDO]` Queda validado el mapeo y recorrido funcional Vb0..Vb6. La
  calibración fina sigue pendiente con una alimentación limpia.
- Próximo paso mínimo: desconectar el pack y preparar la validación de PWM con
  osciloscopio. El menú actual conecta la salida al pulsar START incluso en
  duty 0 %, por lo que no usarlo hasta definir un montaje y una secuencia
  segura.

### 2026-08-17 - Lectura acumulada de un pack 4S real

- Verificación previa del pack, referido a su negativo: 0,000; 3,930; 8,000;
  11,920 y 15,870 V. Las celdas calculadas son 3,930; 4,070; 3,920 y 3,950 V.
- `[MEDIDO]` Con sólo el conector de balance y el visor ADC: Vb0 0,000 V -> 0;
  Vb1 3,930 V -> 43222; Vb2 8,000 V -> 49608; Vb3 11,920 V -> 51463; Vb4
  15,870 V -> 49286.
- Vb4 responde. Sus cuentas no tienen que superar a Vb3 porque cada entrada
  acumulada posee una escala/divisor diferente.
- Frente al pack 3S, con tensiones físicas informadas iguales en Vb1..Vb3, las
  lecturas bajaron 1149, 622 y 427 cuentas (aproximadamente 2,6 %, 1,2 % y
  0,8 %). El usuario identificó como causa la fuente auxiliar conmutada y su
  ruido; por eso había propuesto realizar estas comprobaciones con baterías.
- Estas lecturas validan presencia, orden y rango de los canales, pero el ruido
  del montaje impide utilizarlas como puntos de calibración fina. No constituyen
  por sí solas evidencia de un problema de asentamiento del multiplexor ADC.
- `[PENDIENTE]` Repetir la calibración con una alimentación suficientemente
  limpia; para el bring-up funcional se puede continuar incrementalmente a 5S.

### 2026-08-17 - Lectura acumulada de un pack 3S real

- Montaje: pack de litio 3S conectado por balance, con bornes principales y
  etapa de potencia sin utilizar; visor `ADC raw test` y mandos de balance ya
  comprobados en bajo.
- `[MEDIDO]` Referido al GND del MCU: Vb0 0,000 V -> ADC 1; Vb1 3,930 V ->
  44371; Vb2 8,000 V -> 50230; Vb3 11,920 V -> 51890.
- Las tensiones diferenciales de las celdas son 3,930 V, 4,070 V y 3,920 V.
  Son coherentes para un pack 3S y los cuatro canales avanzan en orden
  monótono.
- Las cuentas de Vb1 no coinciden con la caracterización anterior de un tap
  aislado a igual tensión. Además de la distinta topología de prueba, la fuente
  conmutada ruidosa impide atribuir esa diferencia al ADC o usarla para
  calibración.
- `[PENDIENTE]` Repetir el procedimiento incrementalmente con un pack 4S,
  verificando primero sus cinco tensiones acumuladas con multímetro. No
  habilitar PWM, bornes principales ni balanceadores.

### 2026-08-17 - Vb2 responde, prueba de tap aislado no calibrable

- `[MEDIDO]` Con B7 unido a GND, B6 flotante y fuente limitada aplicada sólo a
  B5: 0 V -> ADC Vb2 3001; 4,209 V -> 25940.
- La pendiente entre esos puntos es 5449,988 cuentas/V. Extrapolar a 7,524 V
  predice aproximadamente 44007, lejos de las 49842 cuentas observadas con la
  batería 2S y B6 conectado.
- La discrepancia demuestra que los taps y sus redes interactúan; una entrada
  individual con las demás flotantes valida que el canal responde, pero no es
  un montaje válido para calibración.
- `[PENDIENTE]` No elevar más B5 con B6 flotante. Construir un simulador 2S
  pasivo con dos resistencias iguales para presentar simultáneamente B7=0,
  B6=V/2 y B5=V, con fuente limitada.

### 2026-08-17 - Vb1 caracterizado

- `[MEDIDO]` Cuatro puntos con fuente referenciada a GND y 0 mA indicados:
  0,000 V -> 3105; 0,998 V -> 14975; 3,003 V -> 38700; 4,209 V -> 53325.
- El ajuste global da 11914,653 cuentas/V y offset 3071,425 cuentas. El residuo
  máximo equivale a 12,7 mV; no se observa saturación a 4,209 V.
- La calibración heredada de Vb1 no representa esta unidad. Conservar como
  candidatos de calibración los extremos medidos, sin escribir EEPROM todavía.
- La prueba superó ligeramente el máximo solicitado de 4,1 V; no aplicar más
  tensión a Vb1.
- Próximo paso mínimo: retornar la fuente a 0 V, mover sólo el positivo desde
  B6/Vb1 hacia B5/Vb2 y caracterizar Vb2 con corriente limitada.

### 2026-08-17 - Vb1 con fuente limitada

- Montaje: fuente auxiliar de dos terminales, negativo unido a GND de entrada y
  X6/B7, positivo a X6/B6, límite de 2 mA; bornes principales y batería real
  desconectados.
- `[MEDIDO]` Vb0 permaneció 0 -> ADC 0. Vb1: 0,998 V -> 14975 y 3,003 V ->
  38700. La fuente indicó 0 mA en ambos puntos, confirmando bleed apagado.
- El ajuste preliminar entre ambos puntos da 11832,918 cuentas/V y offset
  3165,748 cuentas. Ese ajuste predice 47113 a 3,714 V, cercano a las 47033
  cuentas obtenidas con la batería real.
- `[MEDIDO]` Corrección del dato informado: con la misma fuente conectada y B6
  en 0 V, Vb1 mostró ADC 3105, no 3. Esto confirma un offset real cercano a las
  3166 cuentas estimadas con los puntos de 1 y 3 V.
- `[PENDIENTE]` Caracterizar Vb1 en su rango útil de celda de litio (3–4,2 V),
  donde importa la calibración. Medir un punto cercano a 4 V con el mismo
  montaje y límite de corriente.

### 2026-08-17 - Primera lectura de balance 2S

- `[MEDIDO]` Se informaron: Vb0 físico -0,21 V -> ADC 0; Vb1 físico 3,714 V
  -> ADC 47033; Vb2 físico 7,524 V -> ADC 49842.
- Vb1 y Vb2 responden y quedan dentro de rango. Sus cuentas no son directamente
  comparables porque las entradas acumuladas usan divisores diferentes.
- Vb0 negativo se recorta a cero por ser una entrada ADC unipolar. El valor
  indica que el negativo del simulador quedó flotante respecto del GND lógico,
  a diferencia del montaje 0/1/2 V propuesto.
- Montaje aclarado: se usó una batería de litio 2S real, no el simulador; sólo
  estaba conectado el puerto de balance X6/B7..B5 y los bornes principales
  permanecieron desconectados. El multímetro referenciaba al GND del MCU.
- `[MEDIDO]` La corriente informada fue aproximadamente 0,1 mA en la celda 1 y
  0 mA en la celda 2, confirmando que los bleed no estaban activos.
- Las tensiones diferenciales reconstruidas son 3,924 V y 3,810 V. No seguir
  usando celdas de litio reales durante el bring-up; pasar a simulador limitado
  antes de conectar más taps.
- `[PENDIENTE]` La calibración heredada no debe usarse para interpretar estos
  datos. El equipo disponible para simular batería es una fuente de laboratorio
  simple de dos terminales, no un simulador multicanal; probar cada entrada de
  balance por separado, referenciada a GND y con corriente limitada.

### 2026-08-17 - Balanceadores apagados y restricción de ST-Link

- `[MEDIDO]` Con alimentación principal y el visor ADC activo, el usuario
  comprobó los seis mandos de balance y todos estaban en 0 V.
- El ST-Link trabaja/referencia a 3,3 V mientras la placa usa 5 V con la fuente
  principal. Para usar ST-Link el usuario debe retirar la alimentación
  principal; no intentar lecturas SWD en vivo con ambas alimentaciones unidas.
- Se descarta la lectura de registros por ST-Link durante funcionamiento normal.
  La validación física de los seis niveles bajos respalda conectar un simulador
  con corriente limitada.
- Próximo paso mínimo: probar únicamente Vb0, Vb1 y Vb2 con 0/1/2 V acumulados,
  sin conectar los bornes principales ni habilitar bleed.

### 2026-08-17 - Sensado Vout+ validado sin corriente

- `[MEDIDO]` La fuente auxiliar aislada indicó 0 mA en todo el rango de cinco
  puntos. Esto confirma que P00 mantuvo la etapa de salida aislada durante la
  prueba y que los valores corresponden al sensado, no a una prueba de potencia.
- Las tensiones diferenciales reconstruidas como `Vout+ - Vout-` fueron 0,990,
  5,005, 10,001, 14,996 y 24,988 V, coherentes con los puntos nominales de la
  fuente auxiliar.
- Vout+ queda validado en linealidad con buck/PWM apagados. Vout- y los canales
  de corriente siguen pendientes bajo caída positiva controlada.
- Próximo paso mínimo: confirmar físicamente los seis mandos de balance en bajo
  antes de conectar un simulador al conector de balance.

### 2026-08-17 - Linealidad de Vout+ medida; montaje por aclarar

- `[MEDIDO]` Se informaron los puntos Vout+: 0,700 V -> 1517; 4,547 V ->
  10016; 9,515 V -> 21007; 14,495 V -> 32028; 24,471 V -> 54129.
- El ajuste da 2213,405 cuentas/V y offset -44,964 cuentas, con residuo máximo
  equivalente a 5,7 mV. Vout+ es lineal en los puntos ensayados.
- Montaje aclarado: la fuente auxiliar era aislada, con su negativo conectado al
  negativo de salida de batería; el multímetro usaba como referencia el GND de
  entrada. Por eso la salida flotante podía llevar Vout- por debajo de VSS con
  buck apagado.
- Vout- físico fue informado entre -0,29 V y -0,517 V, mientras ADC2 permaneció
  en 0. Es coherente con la saturación a cero de un ADC unipolar ante tensión
  negativa; no valida el canal bajo caída positiva en el shunt.
- Sigue sin informarse la corriente de la fuente auxiliar. La prueba llegó por
  iniciativa del usuario a 24,471 V, por encima del único punto bajo inicialmente
  solicitado; no repetir ese extremo cerca de componentes nominales de 25 V.
- `[PENDIENTE]` Validar Vout- y corriente sólo con caída positiva controlada por
  el shunt, después de comprobar PWM y estados transitorios.

### 2026-08-17 - Linealidad de Vin validada

- `[MEDIDO]` Con fuente limitada a 600 mA se registraron tres puntos:
  11,173 V -> 24582; 13,176 V -> 29017; 15,182 V -> 33454.
- El ajuste lineal resulta 2213,020 cuentas/V y offset -143,303 cuentas. Los
  residuos son -0,774, +1,546 y -0,773 cuentas, equivalentes a menos de
  0,7 mV sobre la recta ajustada.
- Esto confirma canal, alineación y excelente linealidad de Vin en ese rango.
  No convierte la calibración heredada en válida; los puntos deben guardarse
  más adelante en la calibración propia de esta unidad.
- Próximo paso mínimo: aplicar una tensión externa baja y limitada a Vout+ con
  la salida cortada, y validar Vout+ sin habilitar PWM.

### 2026-08-17 - Alineación ADC confirmada físicamente

- `[MEDIDO]` Después de flashear la corrección, con entrada real de 12,175 V,
  Vin pasó de 1676 a 26803. El valor teórico por el desplazamiento es
  `1676 << 4 = 26816`; la diferencia es 13 cuentas.
- `[MEDIDO]` En el mismo montaje: Vout+=0, Vout-=1, Ismps=0 e Idischarge=0 en
  promedio. Son cuentas ADC normalizadas, no unidades eléctricas calibradas.
- Esto valida la alineación de ADCR y el recorrido básico de esos canales con
  señales en reposo. No valida todavía las escalas, ganancias ni polaridades
  bajo tensión/corriente aplicada.
- Próximo paso mínimo: comprobar linealidad de Vin en dos tensiones de entrada
  cercanas y seguras, medidas simultáneamente con multímetro.

### 2026-08-17 - Alineación del resultado ADC corregida

- Montaje: entrada real de 12,175 V, fuente limitada a 600 mA, salida y balance
  desconectados, visor en promedio.
- `[MEDIDO]` Antes de corregir, Vin mostraba 1676; Vout+, Vout-, Ismps e
  Idischarge mostraban 0.
- `[MANUAL]` El capítulo 11.2.7 indica que ADCR contiene el resultado de 12 bits
  en `ADCR[11:0]` y que los bits altos son cero en modo selección. El driver lo
  describía y utilizaba incorrectamente como alineado a la izquierda.
- Se normalizó cada muestra con máscara `0x0FFF` y desplazamiento de cuatro
  bits para llevarla a la escala de 16 bits esperada por el core.
- `[COMPILA]` El target aislado compiló con la normalización corregida: 31.900
  bytes de texto, 32 de data y 3.244 de BSS.
- `[MEDIDO]` La corrección fue flasheada con `cms32_flash_safe`. Firmware de
  31.932 bytes, SHA-256
  `534244C2862C2749F19192CFEDA50BF434AAA682C0E5A4B5E95EBAE421BE5F8A`.
- OpenOCD verificó firmware y EEPROM; el backup de 1024 bytes conserva el
  SHA-256 `0420E37F5266CD02128B508ABF50BAC8A41EC227D1897FC28B69607EE731BE54`.
- `[PENDIENTE]` Repetir la línea base. El valor Vin todavía no debe usarse como
  calibración.

### 2026-08-17 - Estado seguro confirmado antes de medir ADC

- `[MEDIDO]` En `ADC raw test`, con referencia en el negativo de entrada DC,
  P00 mide 3,3 V y P15 mide 0 V.
- P00 alto corresponde a salida de batería desconectada; P15 bajo confirma que
  no hay mando PWM en continua. Falta comprobar con osciloscopio la ausencia de
  pulsos/glitches antes de ensayar potencia.
- Próximo paso mínimo: registrar Vin, Vout+, Vout-, Ismps e Idischarge en modo
  promedio, sin salida ni balance conectados.

### 2026-08-17 - Refresco del visor ADC validado en pantalla

- `[MEDIDO]` El usuario recorrió los canales después del segundo flasheo y
  confirmó que los nombres ya no conservan caracteres del canal anterior.
- El visor queda listo para registrar la línea base ADC con salida principal y
  balance desconectados.
- Próximo paso mínimo: confirmar P00 alto y P15 bajo en el visor ADC, y anotar
  los 14 valores promedio sin señales externas aplicadas.

### 2026-08-17 - Corrección del refresco del visor ADC

- `[MEDIDO]` El visor arranca y Vout+ muestra 0 con la salida desconectada.
- Al cambiar de canal, los nombres cortos dejan caracteres del nombre anterior
  porque `lcdPrint()` no completa con espacios el ancho solicitado.
- Se corrigió el visor para rellenar los nueve caracteres reservados al nombre
  del canal.
- `[COMPILA][MEDIDO]` La corrección se compiló y flasheó con
  `cms32_flash_safe`. Firmware de 31.932 bytes, SHA-256
  `B9B1CD78238679FDBF8C365491544B29CDE7096F50A639508596B5C7ED01228C`.
- El backup previo de 1024 bytes conservó el SHA-256
  `0420E37F5266CD02128B508ABF50BAC8A41EC227D1897FC28B69607EE731BE54` y
  OpenOCD confirmó que coincide con la EEPROM después de programar.
- La corrección fue posteriormente verificada en pantalla.

### 2026-08-17 - Primer flasheo seguro con preservación EEPROM

- `[MEDIDO]` Se ejecutó `cms32_flash_safe` mediante ST-Link V2J46S7 y SWD
  dapdirect, con tensión de target de 3,268 V.
- Firmware de prueba ADC: 31.920 bytes, SHA-256
  `8374EB33997A4C668FD8D6823B943C6C693735FB0B839F66CFA3AE8227508213` y
  option bytes `EE 36 E0`. La herramienta confirmó que no invade `0xFC00`.
- Backup previo guardado en
  `local-backups/cms32l051/eeprom-2026-08-17-before-adc-test.bin`, 1024 bytes,
  SHA-256
  `0420E37F5266CD02128B508ABF50BAC8A41EC227D1897FC28B69607EE731BE54`.
- El firmware fue borrado y programado completamente. OpenOCD verificó la
  imagen y confirmó `EEPROM backup matches flash` antes de `reset run`.
- No se habilitaron desde el procedimiento las salidas de potencia.
- `[PENDIENTE]` Confirmar en la placa que arranca, entrar a `ADC raw test` y
  registrar los 14 canales con la potencia desconectada.

### 2026-08-17 - Visor ADC seguro preparado

- Se agregó temporalmente `Options -> ADC raw test` al firmware normal del
  target CMS32L051; no se creó un target auxiliar.
- El visor recorre Vout+, Vout-, Ismps, Idischarge, temperatura interna y
  externa, Vin y Vb0..Vb6. START alterna promedio/lectura instantánea,
  INC/DEC cambia de canal y STOP sale.
- Al entrar y salir fuerza PWM apagado, balanceadores apagados y salida de
  batería desconectada. `AnalogInputs::powerOn(false)` inicia las conversiones
  sin habilitar P00.
- `[COMPILA]` El target aislado compiló: 31.888 bytes de texto, 32 bytes de
  data, 3.244 bytes de BSS y `.bin` de 31.920 bytes; no invade `0xFC00`.
- Posteriormente fue flasheado con autorización y preservación EEPROM
  verificada. Aún falta probar el visor en la pantalla.

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
