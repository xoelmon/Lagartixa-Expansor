# LARGARTIXA SOUNDLAB EXPANSOR — espaciador estéreo y control de fase (VST3)

Procesado (en este orden):
swap / invertir polaridad -> giro de fase R vs L -> M/S con ancho por 3 bandas -> Espacio + Haas -> mezcla -> ganancia.

* **Width bands:** ancho estéreo (0-200 %) independiente para Low / Mid / High, con crossovers Linkwitz-Riley de 4º orden
  (X-Over 1: 60-400 Hz, X-Over 2: 1-12 kHz). Arrastra las líneas verticales y la altura de cada banda en la pantalla,
  o usa los knobs. Con Low = 0 % tienes graves en mono. Doble clic en una banda = 100 %.
* **Space:** difusión con all-passes de Schroeder. Toma el Mid (sin graves, ver Focus) y lo reparte en fase hacia el Side.
* **Haas:** el Mid retrasado (0,5-30 ms) se inyecta en el Side (efecto Haas en versión compatible con mono).
  Space y Haas, igual que el ancho, solo tocan el Side: **la suma a mono (L+R) no cambia**, así que no hay cancelaciones.
* **Phase:** Rotation gira la fase de R respecto a L (-180 a +180 grados) igual a TODAS las frecuencias (red all-pass sin latencia,
  error <1 grado). Invert L / Invert R invierten polaridad; Swap L/R los intercambia. (La rotación sí cambia la suma a mono.)
* **Output:** Mix dry/wet, ganancia y MONO CHECK (escucha la suma a mono para comprobar compatibilidad).
* **Stereo field:** vectorscopio (vertical = Mid, horizontal = Side; línea vertical = mono, línea horizontal = fases opuestas),
  medidores L/R/M/S, correlación de fase (+1 mono, 0 sin relación, -1 fases opuestas) y ancho estéreo en %.

Latencia: 0 muestras (el plugin no retrasa la señal).
Con todo neutro (anchos 100 %, Space/Haas a 0, Rotation 0) el plugin no cambia ni el nivel ni la fase entre canales.

## Compilar con GitHub (repositorio nuevo, independiente del otro plugin)
1. Crea un repositorio nuevo, por ejemplo `largartixa-expansor`.
2. Add file -> Upload files: sube TODOS los archivos de esta carpeta menos `build.yml` y `LEEME.md`
   (CMakeLists.txt, DSP.h, Params.h, PluginEditor.cpp, PluginEditor.h, PluginProcessor.cpp, PluginProcessor.h, UI.h),
   sueltos en la raíz, sin carpetas.
3. Add file -> Create new file. Como nombre escribe `.github/workflows/build.yml` y pega dentro el contenido de `build.yml`.
4. Commit. En Actions, "Compilar EXPANSOR (Windows)" tarda 10-20 minutos. Al terminar, descarga el artefacto `EXPANSOR-Windows`.
5. Copia la carpeta `LARGARTIXA SOUNDLAB EXPANSOR.vst3` a `C:\Program Files\Common Files\VST3\` y reescanea los plugins.

Licencia: JUCE es AGPLv3 o de pago; si vendes el plugin sin publicar el código necesitas licencia comercial.
