# 8. Ideas de carcasa (ESP32-P4-WIFI6-Touch-LCD-7B)

## Datos de la placa (de la ficha y la wiki de Waveshare)
- Dimensiones exteriores: **99.17 × 164.28 mm** (según un resumen de la ficha; **verificar con el plano de la wiki**). Grosor: no publicado → medir con calibrador.
- Área visible: 86.42 × 154.58 mm. Pantalla 7" IPS 1024×600, táctil capacitivo de 5 puntos.
- Interfaces que necesitan acceso: USB-A (host, cámara), USB-C (programación), microSD, interruptor ON/OFF, botones RESET y BOOT, conector PH2.0 de 12 pines (GPIO y alimentación), altavoz, LED de estado.
- No hay datos publicados de agujeros de fijación ni de la posición física de cada conector: **medir la placa real** antes de diseñar.

## Ideas
| # | Concepto | Ventajas | Cuidados |
|---|----------|----------|----------|
| 1 | **Caja de control integrada** (ABS/PETG): tapa con ventana para la pantalla; dentro, fuente RD-65A, relé, bornera y fusible | Una sola pieza, cableado protegido, fácil de instalar | Ventilar la fuente (ranuras laterales); prensaestopas para cables; separar AC de CC |
| 2 | **Carcasa de dos piezas impresa en 3D** (bisel frontal + tapa trasera) con base inclinada | Barata, personalizable, cortes exactos para USB-A, USB-C, microSD | Medir la placa; dejar acceso a BOOT/RESET; imprimir en PETG o ABS |
| 3 | **Soporte inclinado de escritorio** (30-45°) junto a la cámara | Operador ve el estado sin agacharse; fácil de mover | Cable USB corto a la cámara; base pesada o antideslizante |
| 4 | **Brazo articulado / soporte VESA 75** | Posición ajustable | Evitar vibración de la prensa |

**Recomendación:** idea 1 (caja integrada) con la pantalla en la tapa, y la placa sujeta con un marco impreso en 3D o separadores. La cerradura y el relé quedan junto a la prensa (cable de 12 V) o dentro de la caja si la distancia es corta.

## Requisitos de diseño
- Ventanas para: USB-A (cámara), USB-C (programar), microSD, interruptor, BOOT/RESET.
- Ranuras de ventilación frente a la fuente; no tapar la rejilla de la fuente (129 × 98 × 38 mm).
- Prensaestopas para: cable AC (con alivio de tensión), cable USB del microscopio, cable de 12 V a la cerradura.
- Tapa de bornes de la fuente cubierta; AC separado de CC.
- Antena Wi-Fi: no cubrir con metal.
- Materiales: ABS o PETG (más resistente que PLA al calor de la fuente).

## Pendientes
- [ ] Medir la placa real (largo, ancho, grosor, posición de conectores y agujeros).
- [ ] Decidir entre caja integrada (1) y carcasa de dos piezas (2).
- [ ] Definir dónde queda la caja respecto a la cámara y la prensa.
