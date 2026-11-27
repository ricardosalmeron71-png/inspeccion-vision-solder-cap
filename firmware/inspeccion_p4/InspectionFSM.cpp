#include "InspectionFSM.h"

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
