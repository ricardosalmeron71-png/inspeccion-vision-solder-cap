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
    A[Colocar pieza bajo la cámara] --> B[Revisar soldadura]
    B --> C{Soldadura OK?}
    C -- no --> NG[NG rojo, bloqueo, contador NG]
    C -- sí --> D[Desbloquear prensa, tiempo límite]
    D --> E[Mover a la prensa y presionar tapa]
    E --> F[Regresar pieza bajo la cámara]
    F --> G{Tapa OK?}
    G -- sí --> OK[OK verde, contador OK]
    G -- no --> NG
    E -. no regresa a tiempo .-> NG
    OK -->|pieza retirada| A
    NG -->|reinicio de supervisor| A
```
Reglas: tiempo límite de desbloqueo; plazo de retorno de la pieza; reinicio obligatorio después de NG; la clase `vacio` indica nido libre (pieza retirada) y controla la espera.

## Estados del firmware
```mermaid
stateDiagram-v2
    [*] --> ESPERANDO
    ESPERANDO --> REVISANDO: pieza detectada (no vacio)
    REVISANDO --> DESBLOQUEADO: sold_ok estable (relé ON)
    REVISANDO --> NG: sin_sold estable
    DESBLOQUEADO --> ESPERANDO: vence T_DESBLOQ con la pieza aún en la cámara
    DESBLOQUEADO --> EN_PRENSA: pieza retirada (vacio)
    EN_PRENSA --> VERIFICANDO: pieza regresa
    EN_PRENSA --> NG: vence T_RETORNO
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

## Imágenes de los diagramas
![Estaciones](img/estaciones.png)

![Flujo de decisión](img/flujo.png)

![Máquina de estados](img/estados.png)
