# 7. Seguimiento

## Siguiente paso
Tomar las fotos de entrenamiento mientras llega la placa.

## Pendientes
- [ ] Confirmar variante del chip ("postv3") antes de cargar ejemplos.
- [ ] Definir GPIO del relevador.
- [ ] Probar USB y HDMI simultáneos en el microscopio.
- [ ] Confirmar que el DFR0017 activa con 3.3 V.
- [ ] Medir +12 V en reposo (carga mínima 56 Ω / 5 W si > 13 V).
- [ ] Consultar con Schmidt y con seguridad e higiene (CE, control a dos manos).
- [ ] Decidir ruta Arduino vs ESP-IDF.
- [ ] Definir criterios de éxito con calidad.
- [ ] Comprar materiales (DigiKey / Steren / placa).
- [ ] Capturar fotos de entrenamiento y entrenar modelo.

## Riesgos
| Riesgo | Mitigación |
|--------|-----------|
| Cerradura no soporta ~200 N | Solo prueba de concepto con tope rígido; evaluar solución real con Schmidt |
| No es dispositivo de seguridad certificado | Declarar alcance; consultar seguridad e higiene |
| Falso OK del modelo | Umbral alto, N frames, prueba con lote separado |
| Retraso de la placa (30-60 días) | Capturar fotos y entrenar mientras tanto |
| Cambios de luz/ángulo | Montaje rígido, luz controlada |
| Fuente sin carga mínima | Medir 12 V; resistencia de carga |
| USB+HDMI simultáneos desconocido | Probar; usar solo USB |

## Bitácora
| Fecha | Avance |
|-------|--------|
| 2026-11-08 | Documentación inicial, diagramas y presentación en el repo |
| 2026-11-08 | Presentación (PPTX/PDF) con formato Amphenol RF, imágenes y diagramas |
