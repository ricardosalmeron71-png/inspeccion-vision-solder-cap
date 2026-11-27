# 5. Guía de programación

## Entorno
- PC personal (no la de la empresa).
- Pantalla y táctil: **Arduino IDE** con ejemplos Waveshare.
- Cámara USB (UVC): **ESP-IDF**, componente `usb_host_uvc`, ejemplo `usb_camera_lcd_display`.
- Repo base: https://github.com/waveshareteam/ESP32-P4-WIFI6-Touch-LCD-7B
- Placa: https://www.waveshare.com/esp32-p4-wifi6-touch-lcd-7b.htm

## Pasos
1. **Confirmar la variante del chip** ("postv3") antes de cargar ejemplos.
2. Cargar ejemplos de pantalla/táctil en Arduino IDE y verificar.
3. Cargar `usb_camera_lcd_display` en ESP-IDF y ver el video del microscopio.
4. Exportar el modelo de Edge Impulse (biblioteca C++) e integrarlo.
5. Agregar máquina de estados, relevador y contadores.

## Decisión abierta: Arduino vs ESP-IDF
Como la cámara USB se resuelve con ESP-IDF y tu experiencia es en Arduino, hay dos rutas: (a) proyecto completo en ESP-IDF, o (b) Arduino como componente de ESP-IDF. **Pendiente de evaluar** al llegar la placa; no está verificado cuál funciona con el P4.

## Pines
| Función | GPIO |
|---------|------|
| Relevador IN | **POR DEFINIR** (pin libre del conector PH2.0) |

## Máquina de estados (pseudocódigo)
```
estado = ESPERANDO; relevador = OFF  // bloqueado
loop:
  clase, confianza = inferir(frame)    // la cámara mira siempre la plantilla
  ESPERANDO:    si clase != vacio por N frames -> REVISANDO
  REVISANDO:    si sold_ok estable -> relevador ON, t0 = ahora, DESBLOQUEADO
                si sin_sold estable -> NG
  DESBLOQUEADO: si clase == vacio (pieza llevada a la prensa) -> EN_PRENSA
                si ahora - t0 > T_DESBLOQ -> relevador OFF, ESPERANDO
  EN_PRENSA:    si ahora - t0 > T_DESBLOQ -> relevador OFF
                si clase != vacio por N frames (pieza regresó) -> relevador OFF, VERIFICANDO
                si ahora - t0 > T_RETORNO -> relevador OFF, NG
  VERIFICANDO:  si tapa_ok estable -> OK
                si tapa_mal / sold_ok (sin tapa) estable -> NG
  OK:           contador_ok++; al ver vacio -> ESPERANDO
  NG:           relevador OFF; contador_ng++; esperar reinicio supervisor -> ESPERANDO
```

## Reglas de diseño
- Relevador OFF = bloqueado (la cerradura sin energía queda bloqueada): cualquier falla de energía/firmware deja bloqueado.
- Decidir solo con N frames consecutivos por encima del umbral (anti-parpadeo).
- `T_DESBLOQ`: tiempo límite de desbloqueo (debe alcanzar para mover la pieza a la prensa y presionar; a calibrar).
- `T_RETORNO`: plazo para que la pieza regrese bajo la cámara; si vence, NG.
- Reinicio de supervisor tras NG (botón táctil con código/mantener presionado).
- Interfaz: franja de estado (esperando, revisando, OK, NG), video en vivo y contadores.
- Opcional: servir una página web propia o enviar datos (sin internet; red de planta solo con permiso de IT).

## Parámetros a calibrar
`UMBRAL_CONF`, `N_FRAMES`, `T_DESBLOQ`, `T_RETORNO`, brillo/luz del microscopio.

## Qué programa usar (actualizado con la wiki y el GitHub de Waveshare)
**¿Se puede usar Arduino?** Sí, para la pantalla y el táctil. Waveshare publica **12 sketches Arduino** (pantalla 7B, táctil GT911, Wi-Fi por el ESP32-C6, cámara, microSD, audio, RS485 y CAN) y declara:
- Paquete **Arduino-ESP32 3.3.11**, placa **"ESP32P4 Dev Module"**, **PSRAM habilitada**.
- "Chip Variant" según la revisión del silicio (su CI usa `postv3`, Rev 3.x). Esto confirma el pendiente de la variante del chip.
- No pude leer los nombres de los sketches ni las librerías (esa página devolvió error): se verán en la carpeta `examples/arduino/` del repositorio.

**¿Y la cámara USB y el modelo?** Aquí está la dificultad:
- El repositorio de Waveshare trae 18 ejemplos **ESP-IDF** (v5.5.5), entre ellos `08_lvgl_display_panel` y `09_lvgl_demo_v9`, pero **no incluye ningún ejemplo de cámara USB UVC**. El ejemplo `usb_camera_lcd_display` y el componente `usb_host_uvc` no son de Waveshare (parecen ser de Espressif; **sin verificar** que funcionen en esta placa).
- Tampoco está verificado que Edge Impulse exporte y ejecute el modelo en el ESP32-P4.

**Ruta recomendada**
1. **Etapa 1 (Arduino, sin cámara):** probar pantalla, táctil y relé con los ejemplos de Waveshare, y la lógica del ciclo en `firmware/` con la **clase simulada** (por serial o con botones táctiles). Así el flujo OK/NG se valida antes de tener cámara.
2. **Etapa 2 (ESP-IDF):** partir de `08_lvgl_display_panel` o `09_lvgl_demo_v9`, agregar el componente de cámara UVC y el modelo exportado. Si prefieres seguir en Arduino, habría que probar si el componente UVC se puede usar desde Arduino-ESP32 (no verificado).
3. **Etapa 3:** integrar `InspectionFSM` (ya probada) con el clasificador y la interfaz.

Código: ver [`firmware/`](../firmware/README.md). La máquina de estados implementa exactamente el pseudocódigo de arriba, con estas reglas adicionales probadas en PC: tras vencer el desbloqueo o tras un NG se exige **retirar la pieza** (`vacio`) antes de aceptar otra; confianza baja o parpadeo de clases nunca desbloquea; el relé solo está ON en `DESBLOQUEADO` y `EN_PRENSA`.

**Pines conocidos usados por la placa (wiki):** I2C SCL=8, SDA=7; I2S 9-13 y 53; microSD 39-44; RS485 RXD=27, TXD=26; CAN RXD=21, TXD=22. La tabla de los 17 GPIO del conector de 12 pines **no aparece en la wiki**: consultar el esquemático para elegir `PIN_RELE`.
