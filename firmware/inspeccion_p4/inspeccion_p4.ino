// Esqueleto para ESP32-P4 7B (Arduino-ESP32 3.3.11, placa "ESP32P4 Dev Module", PSRAM habilitada).
// QUÉ HACE: usa la máquina de estados de InspectionFSM y maneja el relé.
// QUÉ NO HACE TODAVÍA: dibujar en la pantalla ni leer la cámara ni clasificar.
//   - Pantalla/táctil: copiar de los ejemplos Arduino de Waveshare e implementar mostrarUI().
//   - Cámara + modelo: ver docs/05-guia-programacion.md (la cámara USB UVC se resuelve con ESP-IDF).
// MODO SIMULACIÓN: por el monitor serie escribe una letra para simular la clase detectada:
//   v=vacio  s=sin_sold  o=sold_ok  t=tapa_ok  m=tapa_mal  r=reinicio de supervisor
#include "InspectionFSM.h"

// ---- AJUSTAR ----
static const int  PIN_RELE        = -1;     // GPIO libre del conector PH2.0 (POR DEFINIR en el esquemático)
static const bool RELE_ACTIVO_ALTO = true;  // módulo con puente en H (alto)
// -----------------

static Config cfg;                           // valores por defecto de InspectionFSM.h
static InspectionFSM fsm(cfg);
static Clase claseSim = Clase::VACIO;
static bool  relePrevio = false;

static void escribirRele(bool desbloquear) {
  if (PIN_RELE < 0) return;
  digitalWrite(PIN_RELE, (desbloquear == RELE_ACTIVO_ALTO) ? HIGH : LOW);
}

static const char* nombreEstado(Estado e) {
  switch (e) {
    case Estado::ESPERANDO:    return "ESPERANDO";
    case Estado::REVISANDO:    return "REVISANDO";
    case Estado::DESBLOQUEADO: return "DESBLOQUEADO";
    case Estado::EN_PRENSA:    return "EN PRENSA";
    case Estado::VERIFICANDO:  return "VERIFICANDO";
    case Estado::OK:           return "OK";
    default:                   return "NG";
  }
}

// Sustituir por LVGL / el código de pantalla de Waveshare: franja de estado, video, contadores.
static void mostrarUI() {
  static Estado previo = Estado::NG; static bool primera = true;
  if (primera || fsm.estado() != previo) {
    primera = false; previo = fsm.estado();
    Serial.printf("[UI] %s  OK:%u  NG:%u  rele:%s\n", nombreEstado(fsm.estado()),
                  (unsigned)fsm.contadorOK(), (unsigned)fsm.contadorNG(), fsm.relayOn() ? "ON" : "OFF");
  }
}

void setup() {
  Serial.begin(115200);
  if (PIN_RELE >= 0) { escribirRele(false); pinMode(PIN_RELE, OUTPUT); escribirRele(false); }  // bloqueado desde el arranque
  Serial.println("Simulacion: v s o t m r");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'v') claseSim = Clase::VACIO;
    else if (c == 's') claseSim = Clase::SIN_SOLD;
    else if (c == 'o') claseSim = Clase::SOLD_OK;
    else if (c == 't') claseSim = Clase::TAPA_OK;
    else if (c == 'm') claseSim = Clase::TAPA_MAL;
    else if (c == 'r') fsm.reinicioSupervisor();
  }
  // Aquí irá: clase = clasificar(frame_de_la_camara); por ahora se usa la clase simulada.
  static uint32_t ultimo = 0;
  uint32_t ahora = millis();
  if (ahora - ultimo >= 100) {            // ~10 cuadros por segundo simulados
    ultimo = ahora;
    fsm.update(ahora, claseSim, 0.99f);
  }
  if (fsm.relayOn() != relePrevio) { relePrevio = fsm.relayOn(); escribirRele(relePrevio); }
  mostrarUI();
}
