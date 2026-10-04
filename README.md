![Samurai Warriors 2: Xtreme Legends Recompiled Logo](sw2-recomp-logo.png)

# sw2xl-recomp

Recompilación para PC de **Samurai Warriors 2: Xtreme Legends** (USA/Europe) con ReXGlue SDK 0.10.0. El proyecto arranca el juego base con **Title Update #3** y carga `SW2XL_US.dll` como módulo XL. El objetivo es jugar XL en PC; el juego base y la actualización son requisitos técnicos de esa ejecución.

Los identificadores generados `samurai_warriors_2` y `samurai_warriors_2_SW2XL_US` se conservan porque forman parte de la ABI y del código generado de ReXGlue. El nombre público del repositorio, el bundle y la documentación es `sw2xl-recomp`.

## Requisitos

- ReXGlue SDK 0.10.0 para Windows AMD64 o Linux AMD64.
- Visual Studio 2026 Community con C++ (Windows), LLVM/Clang, CMake 3.25+ y Ninja.
- Una extracción legal de **Samurai Warriors 2 (USA, Europe)**.
- Los archivos de **Title Update #3** correspondientes a esa versión.
- `SW2XL_US.dll` y paquetes STFS (`LIVE`, `PIRS` o `CON`) de una copia legal de **Samurai Warriors 2: Xtreme Legends**.

## Estructura requerida

Renombra el directorio de este repositorio a `sw2xl-recomp` y déjalo junto al SDK y los archivos extraídos:

```text
rexglue-sdk-win-amd64/
├── bin/
├── include/
├── lib/
├── Samurai Warriors 2 (USA, Europe)/
│   ├── default.xex                         # juego base
│   ├── SW2XL_US.dll                        # módulo XL
│   └── Samurai Warriors 2 Title Update #3/
│       ├── default.xex                      # copia sin modificar del ejecutable base
│       └── default.xexp                     # parche extraído del contenedor TU
└── sw2xl-recomp/
```

No reemplaces el `default.xex` de la raíz: el manifiesto conserva el juego base como raíz de datos y selecciona el `default.xex` junto a `default.xexp` para generar y cargar Title Update #3.

## Preparar juego, actualización y XL

1. Extrae el juego base en `Samurai Warriors 2 (USA, Europe)`.
2. Prepara Title Update #3 siguiendo los pasos de la siguiente sección.
3. Copia `SW2XL_US.dll` a la raíz de `Samurai Warriors 2 (USA, Europe)`.
4. Conserva los contenedores XL STFS en cualquier carpeta local; se importan después de compilar y no se modifican en origen.

### Preparar Title Update #3 desde el paquete original

La actualización original se distribuye como un único archivo, `TU_15KU1UL_000000C000000.00000000000O3`. Es un contenedor STFS con firma `LIVE`; el parche `default.xexp` está dentro. El `default.xex` necesario para este proyecto se obtiene del juego base.

1. Abre el archivo `TU_15KU1UL_000000C000000.00000000000O3` con una herramienta de extracción STFS, como [Velocity](https://github.com/hetelek/Velocity), y extrae `default.xexp`.
2. Crea `Samurai Warriors 2 Title Update #3` dentro de `Samurai Warriors 2 (USA, Europe)`.
3. Copia el `default.xex` original del juego base a esa subcarpeta y coloca junto a él el `default.xexp` extraído.

ReXGlue busca el archivo hermano `default.xexp` y aplica el parche en memoria al cargar `default.xex`. Conserva esa copia del ejecutable sin modificar: no apliques el parche previamente con otra herramienta. Mantén también el ejecutable original en la raíz del juego.

Copiar únicamente el contenedor `TU_…` a esa subcarpeta no prepara la actualización. `install-dlc.ps1` / `install-dlc.sh` importan contenido XL a `userdata`; no realizan esta preparación del ejecutable y su parche.

## Compilar y ejecutar (Windows)

Desde la raíz del SDK:

```powershell
.\sw2xl-recomp\build.ps1
.\sw2xl-recomp\run.ps1
```

El ejecutable resultante sigue llamándose `samurai_warriors_2.exe` por compatibilidad con el código generado y queda en `sw2xl-recomp/out/build/win-amd64-release/`.

`build.ps1` espera Visual Studio en `C:\Program Files\Microsoft Visual Studio\18\Community`. Ajusta `$vsRoot` si tu instalación está en otra ruta.

El launcher usa ventana, XInput, emulación teclado/ratón, RTV/DSV y rutas locales de datos por defecto. Puedes sobrescribir opciones:

```powershell
.\sw2xl-recomp\run.ps1 --fullscreen
.\sw2xl-recomp\run.ps1 --input_backend sdl --no-mnk_mode
.\sw2xl-recomp\run.ps1 --log_level info
```

## Instalar contenido Xtreme Legends

Primero compila. Luego importa los contenedores STFS de XL a los datos locales:

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

Desde la raíz del SDK:

```bash
./sw2xl-recomp/build.sh
./sw2xl-recomp/install-dlc.sh --dlc-root '/path/to/Samurai Warriors 2 XL'
./sw2xl-recomp/run.sh
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
| `run.ps1` / `run.sh` | Lanzador con rutas de datos locales |
| `install-dlc.ps1` / `install-dlc.sh` | Importación de contenido XL STFS |
| `package.ps1` / `package.sh` | Bundle portátil de pruebas XL |
| `samurai_warriors_2_manifest.toml` | Entrypoint actualizado y módulo XL |
| `sw2xl_us_config.toml` | Configuración de funciones del módulo XL |
| `src/dlc_installer.cpp` | Instalador de contenido en tiempo de ejecución |
