# Firmware

| Carpeta | Contenido | Estado |
|---------|-----------|--------|
| `inspeccion_p4/InspectionFSM.*` | Máquina de estados del ciclo (sin dependencias de hardware) | **Probada en PC** (`tests/`) |
| `inspeccion_p4/inspeccion_p4.ino` | Esqueleto Arduino: relé + modo simulación por serial | Solo revisada la sintaxis; **no probada en la placa** |
| `tests/test_fsm.cpp` | Pruebas del ciclo: OK, sin soldadura, tapa mal, pieza no regresa, anti-parpadeo, desborde del reloj | Pasan |

## Probar la lógica en una PC
```bash
cd firmware/tests
g++ -std=c++11 -Wall -Wextra -I../inspeccion_p4 test_fsm.cpp ../inspeccion_p4/InspectionFSM.cpp -o test_fsm && ./test_fsm
```

## Qué falta
- Dibujar la interfaz (pantalla): usar los ejemplos de Waveshare e implementar `mostrarUI()`.
- Leer la cámara USB y clasificar con el modelo de Edge Impulse.
- Definir `PIN_RELE` (GPIO libre del conector PH2.0) con el esquemático.
- Calibrar `umbralConf`, `nFrames`, `tDesbloqMs` y `tRetornoMs` con tiempos reales.
