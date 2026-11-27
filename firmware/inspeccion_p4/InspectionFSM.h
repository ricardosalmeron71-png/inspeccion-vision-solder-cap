// Lógica del ciclo de inspección (sin dependencias de Arduino ni de ESP-IDF).
// Se puede compilar y probar en una PC. Ver docs/05-guia-programacion.md.
#pragma once
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
