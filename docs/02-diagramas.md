# 2. Diagramas

## Bloques
```mermaid
flowchart LR
    M[Microscopio USB] -->|USB datos| P[ESP32-P4 7in]
    P -->|GPIO| R[Relevador]
    R -->|12 V| L[Cerradura]
    L -->|tope| PR[Prensa]
    F[Fuente RD-65A] -->|5 V| M
    F -->|5 V| P
    F -->|5 V| R
    F -->|12 V| R
```

## Flujo del ciclo
```mermaid
flowchart TD
    A[Esperando pieza] --> B[Revisar soldadura]
    B --> C{Soldadura OK?}
    C -- sí --> D[Desbloquear prensa unos segundos]
    C -- no --> NG[NG rojo, bloqueo, contador NG]
    D --> E[Operador presiona tapa]
    E --> F{Tapa puesta?}
    F -- sí --> OK[OK verde, contador OK]
    F -- no --> NG
    OK --> A
    NG -->|reinicio de supervisor| A
```
Reglas: tiempo límite (si no se presiona, se vuelve a bloquear); reinicio obligatorio después de NG; la clase `vacio` controla la espera de pieza.

## Estados del firmware
```mermaid
stateDiagram-v2
    [*] --> ESPERANDO
    ESPERANDO --> REVISANDO: pieza detectada (no vacio)
    REVISANDO --> DESBLOQUEADO: sold_ok estable
    REVISANDO --> NG: sin_sold estable
    DESBLOQUEADO --> VERIFICANDO: tapa presionada / pieza cambia
    DESBLOQUEADO --> ESPERANDO: tiempo límite (re-bloqueo)
    VERIFICANDO --> OK: tapa_ok
    VERIFICANDO --> NG: tapa_mal / sin tapa
    OK --> ESPERANDO: pieza retirada (vacio)
    NG --> ESPERANDO: reinicio supervisor
```

## Eléctrico (resumen)
```mermaid
flowchart LR
    F5[+5 V] --> P[Placa pin 5V]
    F5 --> INJ[Inyector microscopio]
    F5 --> RV[Relevador VCC]
    F12[+12 V] --> COMR[Relevador COM]
    COMR --> NO[Relevador NO] --> LP[Cerradura +]
    LM[Cerradura -] --> GND[COM / tierra común]
    GPIO[GPIO PH2.0] --> RIN[Relevador IN]
```
