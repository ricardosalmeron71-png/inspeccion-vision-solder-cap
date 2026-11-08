# 1. Propósito y objetivos

## Propósito
Evitar que una pieza sin soldadura, o sin tapa, avance en el ensamble de la tapa de un conector coaxial angular, usando visión por computadora en un equipo independiente.

## Restricciones
- IT no permite computadora en la línea; sin permisos de administrador en la PC de la empresa.
- Programación desde PC personal.
- Conectar a la red de planta requiere permiso de IT (no es necesario para operar).

## Objetivos
1. Detectar soldadura presente/ausente antes del prensado.
2. Bloquear la palanca de la prensa si no hay soldadura; desbloquear unos segundos si es OK.
3. Verificar la tapa después del prensado.
4. Mostrar estado (esperando, revisando, OK, NG), video en vivo y contadores OK/NG.
5. Exigir reinicio de supervisor después de un NG.
6. Operar sin PC, con una sola fuente y una placa.

## Alcance
**Incluye:** captura, modelo (Edge Impulse), firmware, cerradura de prueba de concepto, interfaz táctil.
**No incluye:** certificación de seguridad (CE), control a dos manos, integración a red/MES.

## Criterios de éxito (por definir con calidad)
- Cero falsos OK en la prueba con lote/día separado (meta a acordar).
- Tiempo de decisión menor a ~1 s (a validar).
- Operador entiende el estado sin capacitación extensa.

## Clases del modelo
`vacio`, `sin_sold`, `sold_ok`, `tapa_ok`, `tapa_mal` (opcional).
