// VERSIÓN DE UN SOLO ARCHIVO: pegar completo en Arduino IDE. No necesita InspectionFSM.h/.cpp.

// ===================== MÁQUINA DE ESTADOS =====================
// Lógica del ciclo de inspección (sin dependencias de Arduino ni de ESP-IDF).
// Se puede compilar y probar en una PC. Ver docs/05-guia-programacion.md.
#include <stdint.h>

enum class Clase : uint8_t { NINGUNA = 0, VACIO, SIN_SOLD, SOLD_OK, TAPA_OK, TAPA_MAL };

enum class Estado : uint8_t {
  ESPERANDO,     // nido libre o esperando pieza; relé OFF (bloqueado)
  REVISANDO,     // evaluando soldadura
  DESBLOQUEADO,  // relé ON, pieza aún bajo la cámara
  EN_PRENSA,     // pieza retirada hacia la prensa
  VERIFICANDO,   // pieza regresó; evaluando tapa
  OK,            // ciclo correcto, espera retirar la pieza
  NG             // rechazo, bloqueado, espera reinicio de supervisor
};

enum class MotivoNG : uint8_t {
  NINGUNO = 0, SIN_SOLDADURA, TAPA_MAL, NO_REGRESO, PIEZA_INESPERADA, SOLDADURA_CAMBIO
};

struct Config {
  float    umbralConf   = 0.85f;   // confianza mínima para aceptar una clase
  uint8_t  nFrames      = 3;       // cuadros consecutivos iguales para dar por estable
  uint32_t tDesbloqMs   = 20000;   // tiempo máximo con el relé ON (a calibrar)
  uint32_t tRetornoMs   = 60000;   // plazo para que la pieza regrese (desde el desbloqueo)
};

class InspectionFSM {
 public:
  explicit InspectionFSM(const Config& c = Config()) : cfg_(c) {}

  // Llamar una vez por cuadro clasificado. now_ms: reloj en milisegundos.
  void update(uint32_t now_ms, Clase clase, float confianza);
  // Llamar periódicamente aunque no haya cuadro nuevo (vence tiempos).
  void tick(uint32_t now_ms);
  // Reinicio de supervisor tras un NG.
  void reinicioSupervisor();

  Estado   estado()  const { return estado_; }
  MotivoNG motivo()  const { return motivo_; }
  bool     relayOn() const { return relay_; }   // true = desbloqueado
  uint32_t contadorOK() const { return ok_; }
  uint32_t contadorNG() const { return ng_; }
  bool     exigeRetirarPieza() const { return needClear_; }

 private:
  Clase estable(Clase clase, float conf);   // devuelve NINGUNA si aún no es estable
  void  irNG(MotivoNG m);
  void  bloquear() { relay_ = false; }
  void  procesar(uint32_t now, Clase est);
  void  vencimientos(uint32_t now);

  Config   cfg_;
  Estado   estado_ = Estado::ESPERANDO;
  MotivoNG motivo_ = MotivoNG::NINGUNO;
  bool     relay_ = false;
  bool     needClear_ = false;   // hay que ver VACIO antes de aceptar otra pieza
  uint32_t t0_ = 0;              // instante del desbloqueo
  uint32_t ok_ = 0, ng_ = 0;
  Clase    cand_ = Clase::NINGUNA;
  uint8_t  candN_ = 0;
};


Clase InspectionFSM::estable(Clase clase, float conf) {
  if (clase == Clase::NINGUNA || conf < cfg_.umbralConf) {   // cuadro dudoso: reinicia la cuenta
    cand_ = Clase::NINGUNA; candN_ = 0; return Clase::NINGUNA;
  }
  if (clase == cand_) { if (candN_ < 255) candN_++; }
  else { cand_ = clase; candN_ = 1; }
  return (candN_ >= cfg_.nFrames) ? cand_ : Clase::NINGUNA;
}

void InspectionFSM::irNG(MotivoNG m) {
  bloquear();
  estado_ = Estado::NG;
  motivo_ = m;
  ng_++;
}

void InspectionFSM::vencimientos(uint32_t now) {
  uint32_t dt = now - t0_;   // seguro ante desborde de uint32
  if (estado_ == Estado::DESBLOQUEADO) {
    if (dt > cfg_.tDesbloqMs) { bloquear(); needClear_ = true; estado_ = Estado::ESPERANDO; }
  } else if (estado_ == Estado::EN_PRENSA) {
    if (dt > cfg_.tDesbloqMs) bloquear();                 // se vuelve a bloquear
    if (dt > cfg_.tRetornoMs) irNG(MotivoNG::NO_REGRESO); // la pieza no regresó
  }
}

void InspectionFSM::procesar(uint32_t now, Clase est) {
  switch (estado_) {
    case Estado::ESPERANDO:
      if (est == Clase::VACIO) { needClear_ = false; break; }
      if (est != Clase::NINGUNA && !needClear_) { estado_ = Estado::REVISANDO; procesar(now, est); }
      break;
    case Estado::REVISANDO:
      if (est == Clase::SOLD_OK) { relay_ = true; t0_ = now; estado_ = Estado::DESBLOQUEADO; }
      else if (est == Clase::SIN_SOLD) irNG(MotivoNG::SIN_SOLDADURA);
      else if (est == Clase::TAPA_OK || est == Clase::TAPA_MAL) irNG(MotivoNG::PIEZA_INESPERADA);
      else if (est == Clase::VACIO) estado_ = Estado::ESPERANDO;
      break;
    case Estado::DESBLOQUEADO:
      if (est == Clase::VACIO) estado_ = Estado::EN_PRENSA;           // la pieza se llevó a la prensa
      else if (est == Clase::SIN_SOLD) irNG(MotivoNG::SOLDADURA_CAMBIO);
      break;
    case Estado::EN_PRENSA:
      if (est != Clase::NINGUNA && est != Clase::VACIO) {            // la pieza regresó
        bloquear(); estado_ = Estado::VERIFICANDO; procesar(now, est);
      }
      break;
    case Estado::VERIFICANDO:
      if (est == Clase::TAPA_OK) { estado_ = Estado::OK; ok_++; motivo_ = MotivoNG::NINGUNO; }
      else if (est == Clase::TAPA_MAL || est == Clase::SOLD_OK) irNG(MotivoNG::TAPA_MAL);
      else if (est == Clase::SIN_SOLD) irNG(MotivoNG::SIN_SOLDADURA);
      break;
    case Estado::OK:
      if (est == Clase::VACIO) { estado_ = Estado::ESPERANDO; needClear_ = false; }
      break;
    case Estado::NG:
      break;   // solo el reinicio de supervisor sale de NG
  }
}

void InspectionFSM::update(uint32_t now_ms, Clase clase, float confianza) {
  vencimientos(now_ms);
  procesar(now_ms, estable(clase, confianza));
}

void InspectionFSM::tick(uint32_t now_ms) { vencimientos(now_ms); }

void InspectionFSM::reinicioSupervisor() {
  if (estado_ != Estado::NG) return;
  bloquear(); estado_ = Estado::ESPERANDO; motivo_ = MotivoNG::NINGUNO;
  needClear_ = true;                     // retirar la pieza rechazada
  cand_ = Clase::NINGUNA; candN_ = 0;
}

// ===================== SKETCH =====================
// Esqueleto para ESP32-P4 7B (Arduino-ESP32 3.3.11, placa "ESP32P4 Dev Module", PSRAM habilitada).
// QUÉ HACE: usa la máquina de estados de InspectionFSM y maneja el relé.
// QUÉ NO HACE TODAVÍA: dibujar en la pantalla ni leer la cámara ni clasificar.
//   - Pantalla/táctil: copiar de los ejemplos Arduino de Waveshare e implementar mostrarUI().
//   - Cámara + modelo: ver docs/05-guia-programacion.md (la cámara USB UVC se resuelve con ESP-IDF).
// MODO SIMULACIÓN: por el monitor serie escribe una letra para simular la clase detectada:
//   v=vacio  s=sin_sold  o=sold_ok  t=tapa_ok  m=tapa_mal  r=reinicio de supervisor

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
