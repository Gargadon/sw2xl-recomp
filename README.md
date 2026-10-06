![Samurai Warriors 2: Xtreme Legends Recompiled Logo](sw2-recomp-logo.png)

# sw2xl-recomp

Recompilación para PC de **Samurai Warriors 2: Xtreme Legends** (USA/Europe) con ReXGlue SDK 0.10.0. El proyecto arranca el juego base con **Title Update #3** y carga `SW2XL_US.dll` como módulo XL. El objetivo es jugar XL en PC; el juego base y la actualización son requisitos técnicos de esa ejecución.

Los identificadores generados `samurai_warriors_2` y `samurai_warriors_2_SW2XL_US` se conservan porque forman parte de la ABI y del código generado de ReXGlue. El nombre público del repositorio, el bundle y la documentación es `sw2xl-recomp`.

## Requisitos

- ReXGlue SDK 0.10.0 para Windows AMD64 o Linux AMD64.
- Visual Studio 2026 Community con C++ (Windows), LLVM/Clang, CMake 3.25+ y Ninja.
- Una extracción legal de **Samurai Warriors 2 (USA, Europe)**.
- El contenedor original de **Title Update #3** correspondiente a esa versión: `TU_15KU1UL_000000C000000.00000000000O3`.
- Una carpeta con los paquetes STFS (`LIVE`, `PIRS` o `CON`) de una copia legal de **Samurai Warriors 2: Xtreme Legends**, incluido el paquete que contiene `SW2XL_US.dll`.

## Estructura requerida

Renombra el directorio de este repositorio a `sw2xl-recomp` y déjalo junto al SDK y los archivos extraídos:

```text
rexglue-sdk-win-amd64/
├── bin/
├── include/
├── lib/
├── Samurai Warriors 2 (USA, Europe)/
│   ├── default.xex                         # juego base original
│   └── data/                               # datos extraídos del juego
└── sw2xl-recomp/
```

No reemplaces el `default.xex` de la raíz: el manifiesto conserva el juego base como raíz de datos y selecciona el `default.xex` junto a `default.xexp` para generar y cargar Title Update #3.

## Compilar (Windows)

1. Extrae el juego base en `Samurai Warriors 2 (USA, Europe)`.
2. Conserva el contenedor original de Title Update #3 y la carpeta de paquetes XL en cualquier ubicación local.
3. Desde el directorio `sw2xl-recomp`, ejecuta el build indicando **ambas rutas**:

```powershell
.\build.ps1 -TitleUpdatePackage 'D:\Samurai Warriors 2 Title Update #3\TU_15KU1UL_000000C000000.00000000000O3' -DlcRoot 'D:\Samurai Warriors 2 XL'
```

`-TitleUpdatePackage` apunta al **archivo** del contenedor TU #3; `-DlcRoot`, a la **carpeta** de paquetes XL. Ambos parámetros son obligatorios. Si falta el archivo del Title Update o la carpeta del DLC, el script se detiene antes de configurar o compilar, incluso si quedan archivos preparados de un build anterior.

El script prepara primero Title Update #3: extrae `default.xexp` y copia el ejecutable base a `Samurai Warriors 2 Title Update #3` dentro del directorio del juego. Después extrae `SW2XL_US.dll` a la raíz del juego e importa todos los paquetes XL a `userdata` con un instalador de consola que no ejecuta el juego. Finalmente ejecuta codegen, reconfigura CMake para incorporar los destinos generados y compila. Los archivos preparados que ya existen se conservan; los paquetes de origen no se modifican. Si la extracción o importación falla, no se continúa con codegen ni con la compilación del juego.

ReXGlue busca el archivo hermano `default.xexp` y aplica el parche en memoria al cargar `default.xex`. Conserva esa copia del ejecutable sin modificar: no apliques el parche previamente con otra herramienta. Mantén también el ejecutable original en la raíz del juego.

El ejecutable resultante sigue llamándose `samurai_warriors_2.exe` por compatibilidad con el código generado y queda en `sw2xl-recomp/out/build/win-amd64-release/`.

`build.ps1` espera Visual Studio en `C:\Program Files\Microsoft Visual Studio\18\Community`. Ajusta `$vsRoot` si tu instalación está en otra ruta.

## Recompilar después de una mejora

Después de ejecutar `build` al menos una vez y preparar TU #3 y XL, usa:

```powershell
.\rebuild.ps1
```

En Linux: `bash ./rebuild.sh`. No necesitas volver a indicar las rutas de los paquetes originales. `rebuild` comprueba que existan la configuración, las listas de código generado y los archivos preparados del juego. Reutiliza las opciones del SDK y PGO guardadas, omite la preparación del TU y XL y recompila únicamente lo que Ninja detecte como modificado, junto con sus dependencias. Codegen solo se repite si cambian sus entradas. No importa contenido a `userdata` ni limpia el build.

## Ejecutar (Windows)

Después de compilar, ejecuta desde el directorio `sw2xl-recomp`:

```powershell
.\run.ps1
```

El launcher usa ventana, XInput, emulación teclado/ratón, RTV/DSV y rutas locales de datos por defecto. Puedes sobrescribir opciones:

```powershell
.\sw2xl-recomp\run.ps1 --fullscreen
.\sw2xl-recomp\run.ps1 --input_backend sdl --no-mnk_mode
.\sw2xl-recomp\run.ps1 --log_level info
```

## Instalar contenido Xtreme Legends

`build` ya importa los contenedores STFS de XL a los datos locales. Para añadir o volver a importar paquetes después, sin iniciar el juego:

```powershell
.\sw2xl-recomp\install-dlc.ps1 --dlc-root 'D:\Samurai Warriors 2 XL'
```

También se acepta la sintaxis nativa de PowerShell:

```powershell
.\sw2xl-recomp\install-dlc.ps1 -DlcRoot 'D:\Samurai Warriors 2 XL'
```

El importador busca contenedores `LIVE`, `PIRS` y `CON`, los instala mediante el gestor de contenido de ReXGlue en `sw2xl-recomp/userdata`, y nunca altera los paquetes de origen. Ejecuta `run.ps1` normalmente después de la importación.

## Datos locales y bundle

| Datos | Ubicación predeterminada |
| --- | --- |
| Partidas, perfiles y contenido XL instalado | `sw2xl-recomp/userdata` |
| Caché de shaders | `sw2xl-recomp/cache` |

Para un bundle de pruebas autocontenido:

```powershell
.\sw2xl-recomp\package.ps1
.\sw2xl-recomp\package.ps1 -Zip
```

El resultado predeterminado es `sw2xl-recomp/dist/sw2xl-test-bundle`.

## Linux

Desde el directorio `sw2xl-recomp`, configura las rutas de ambos paquetes antes de ejecutar el script:

```bash
cmake --preset linux-amd64-release \
    '-DREXSDK_DIR=..' \
    '-DSW2_TU_PACKAGE=/path/to/TU_15KU1UL_000000C000000.00000000000O3' \
    '-DSW2_DLC_ROOT=/path/to/Samurai Warriors 2 XL'
./build.sh
./run.sh
```

El binario Linux es `out/build/linux-amd64-release/samurai_warriors_2`. Hay scripts adicionales para PGO, bundle y comparación de rutas Vulkan: `build-pgo-*.sh`, `run-pgo-training.sh`, `package.sh`, `run-fbo.sh` y `run-fsi.sh`.

## Notas técnicas

- El primer paso hacia el port nativo es una alternativa C++ para la conversión
  de entrada, activable con `SW2_NATIVE_INPUT=1`; `SW2_NATIVE_INPUT=verify` compara
  automáticamente contra la rutina original usando la misma muestra del mando.
  Véase [native-port.md](native-port.md)
  para el alcance, pruebas y comparación con la rutina original.

- La corrección de despacho XL se aplica tras codegen con `patch-xl-codegen.ps1`/`.sh`.
- La sincronización de eventos del juego base y XL está en `src/event_fix.cpp`.
- `samurai_warriors_2_manifest.toml` describe el juego base, la actualización y el módulo XL. No renombres sus destinos generados manualmente.
- `crash-investigation.md` conserva el análisis técnico del arreglo de eventos.

## Archivos principales

| Archivo | Propósito |
| --- | --- |
| `build.ps1` / `build.sh` | Compilación Release de sw2xl-recomp |
| `rebuild.ps1` / `rebuild.sh` | Recompilación incremental sin preparar TU ni XL |
| `run.ps1` / `run.sh` | Lanzador con rutas de datos locales |
| `install-dlc.ps1` / `install-dlc.sh` | Importación de contenido XL STFS |
| `package.ps1` / `package.sh` | Bundle portátil de pruebas XL |
| `samurai_warriors_2_manifest.toml` | Entrypoint actualizado y módulo XL |
| `sw2xl_us_config.toml` | Configuración de funciones del módulo XL |
| `src/dlc_installer.cpp` | Instalador de contenido en tiempo de ejecución |

Para el flujo habitual basta con `build` la primera vez, `rebuild` para las mejoras posteriores y `run` para jugar. `install-dlc` permite importar paquetes adicionales sin iniciar el juego. Los scripts `build-pgo-*` y `run-pgo-training` son opcionales para entrenar PGO; `package` y `run-bundle.sh` sirven para distribuir un bundle. `patch-xl-codegen` se ejecuta desde CMake y sigue siendo necesario. Los lanzadores `run-rtv`, `run-rov`, `run-fbo` y `run-fsi` son atajos opcionales: las mismas opciones se pueden pasar a `run`.
