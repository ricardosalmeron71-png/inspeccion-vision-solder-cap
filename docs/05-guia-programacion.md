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
