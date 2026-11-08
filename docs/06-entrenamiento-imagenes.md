# 6. Entrenamiento con imágenes (Edge Impulse, navegador)

## Clases
| Clase | Qué muestra |
|-------|-------------|
| `vacio` | Nido vacío (controla la espera de pieza) |
| `sin_sold` | Conector sin soldadura |
| `sold_ok` | Soldadura correcta |
| `tapa_ok` | Tapa colocada correctamente |
| `tapa_mal` | Tapa mal colocada/ausente (opcional) |

## Cantidad
80-100 fotos por clase, cantidad similar entre clases.

## Reglas de captura
- Montaje final fijo: misma escuadra, ángulo (30-45°), distancia y zoom.
- Misma resolución que usará la placa (probable 1280x720 o menor por UVC; verificar).
- Luz constante (marcar nivel del LED); tapar luz ambiente.
- **Recolocar la pieza en cada foto.**
- Variar lote, día y operador.
- `vacio`: con y sin mano del operador.
- `sin_sold`: usar piezas malas reales (pedir rechazos a calidad).

## Nombres de archivo
`clase.lote.dia.consecutivo.jpg`, p. ej. `sold_ok.L23.d1.001.jpg`.
En Edge Impulse: *Infer from filename* para etiquetar.

## Validación
Probar con un día o lote **completo separado** (no mezclado con entrenamiento). Registrar matriz de confusión; prioridad: **cero falsos OK** (`sin_sold` clasificado como `sold_ok`).

## Estructura sugerida
```
data/
  train/ vacio, sin_sold, sold_ok, tapa_ok, tapa_mal
  test/  (lote/día separado)
```
> No subir fotos con información confidencial de la empresa sin permiso.

## Registro de capturas
| Fecha | Lote | Operador | Clase | Cantidad | Notas |
|-------|------|----------|-------|----------|-------|
