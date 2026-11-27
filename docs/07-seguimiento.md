# 7. Seguimiento

## Siguiente paso
1. Fabricar o improvisar la plantilla (nido) bajo la cámara.
2. Tomar fotos de prueba y confirmar que la soldadura se ve desde arriba.
3. Tomar las fotos de entrenamiento mientras llega la placa.

## Pendientes
- [ ] Confirmar en el esquemático de Waveshare el voltaje, la corriente y el pinout de alimentación (USB-C y conector de 12 pines). La wiki no muestra un pin 5V.
- [x] Variante del chip: esptool reporta **ESP32-P4 revisión v3.2** → usar Chip Variant `postv3`.
- [ ] Definir GPIO del relevador.
- [ ] Probar USB y HDMI simultáneos en el microscopio.
- [ ] Confirmar que el módulo de relé (Amazon) activa con 3.3 V; puente en alto y resistencia de 10 kΩ.
- [ ] Medir +12 V en reposo (carga mínima 56 Ω / 5 W si > 13 V).
- [ ] Consultar con Schmidt y con seguridad e higiene (CE, control a dos manos).
- [ ] Decidir ruta Arduino vs ESP-IDF (ver guía de programación: etapas 1-3).
- [ ] Verificar que la cámara UVC (usb_host_uvc) y Edge Impulse funcionen en el ESP32-P4.
- [ ] Definir `T_DESBLOQ` y `T_RETORNO` midiendo el tiempo real de mover la pieza y prensar.
- [ ] Fabricar la plantilla de posición bajo la cámara.
- [ ] Confirmar con Schmidt qué pasa si la cerradura se re-bloquea durante la carrera de la palanca.
- [ ] Definir criterios de éxito con calidad.
- [ ] Comprar materiales (DigiKey / Steren / Amazon). Confirmar las especificaciones de los 4 enlaces de Amazon (ver materiales).
- [ ] Capturar fotos de entrenamiento y entrenar modelo.

## Riesgos
| Riesgo | Mitigación |
|--------|-----------|
| Cerradura no soporta ~200 N | Solo prueba de concepto con tope rígido; evaluar solución real con Schmidt |
| No es dispositivo de seguridad certificado | Declarar alcance; consultar seguridad e higiene |
| Falso OK del modelo | Umbral alto, N frames, prueba con lote separado |
| Retraso de la placa (30-60 días) | Capturar fotos y entrenar mientras tanto |
| Cambios de luz/posición | Plantilla de posición, foco y altura marcados, luz controlada |
| El sistema no sabe si la pieza prensada es la inspeccionada | Desbloqueo solo con la pieza retirada de la cámara y tiempo corto; a futuro, sensor o segunda cámara en la prensa |
| Re-bloqueo mientras el operador presiona | Validar con Schmidt; ajustar `T_DESBLOQ` |
| Soldadura poco visible desde arriba | Fotos de prueba antes del entrenamiento |
| Carga baja en la fuente | Datos del fabricante: CH1 > 10 % (cumple, ≈ 35 %); medir 12 V en reposo |
| USB+HDMI simultáneos desconocido | Probar; usar solo USB |

## Bitácora
| Fecha | Avance |
|-------|--------|
| 2026-11-08 | Documentación inicial, diagramas y presentación en el repo |
| 2026-11-08 | Presentación (PPTX/PDF) con formato Amphenol RF, imágenes y diagramas |
| 2026-11-08 | Flujo corregido: cámara fija en su base mirando hacia abajo; pieza se mueve a la prensa y regresa. Se agregó la fuente RD-65A (foto, datos y bornes) |
| 2026-11-08 | Decisión: relevador = módulo de 1 canal 5 V de Amazon (se descarta DFR0017); fuente RD-65A confirmada en Amazon ($384.08) |
| 2026-11-27 | Firmware: máquina de estados probada en PC (9 pruebas) y esqueleto Arduino con modo simulación |
| 2026-11-27 | Placa conectada (COM80). Chip ESP32-P4 rev v3.2. Sketch compila (313 KB, 23 %). Subida falla al cambiar a 921600 baudios: bajar Upload Speed |
| 2026-11-27 | esptool `--no-stub flash-id` OK en COM79 (USB-Serial/JTAG): flash GigaDevice (0xC8), 32 MB. La subida desde Arduino falla con el stub; solución: grabar con `--no-stub write-flash` el binario exportado |
| 2026-11-27 | Grabación falla en ambos USB-C (COM79 USB-Serial/JTAG y COM80 USB-UART), con y sin stub, con flash fijada en 4 MB: se corta al borrar/escribir la flash. Lectura de flash-id sí funciona. Siguiente: actualizar paquete esp32 a 3.3.11 (esptool más nuevo), probar grabador web esptool-js, o soporte Waveshare |
| 2026-11-27 | **Grabación exitosa** con esptool 5.3.1 (descarga independiente, sin instalar): `write-flash 0x0 inspeccion_p4.ino.merged.bin` en COM79, 4 MB en 15.8 s, hash verificado. Causa de los fallos: esptool 5.1.0 del paquete esp32 3.3.3. Pendiente: confirmar que el programa arranca (Monitor Serie) y actualizar el paquete a 3.3.12 |
