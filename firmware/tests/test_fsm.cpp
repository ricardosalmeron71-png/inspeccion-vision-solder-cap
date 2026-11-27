// Pruebas del núcleo en PC:  g++ -std=c++11 -Wall -Wextra -I../inspeccion_p4 test_fsm.cpp ../inspeccion_p4/InspectionFSM.cpp -o test_fsm && ./test_fsm
#include <cstdio>
#include <cstdlib>
#include "InspectionFSM.h"

static int fallos = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FALLO linea %d: %s\n", __LINE__, #c); fallos++; } } while (0)

struct Sim {
  InspectionFSM f; uint32_t t = 0;
  explicit Sim(const Config& c = Config()) : f(c) {}
  void frames(Clase c, int n, float conf = 0.95f, uint32_t dt = 100) { for (int i = 0; i < n; i++) { t += dt; f.update(t, c, conf); } }
  void espera(uint32_t ms) { t += ms; f.tick(t); }
};

int main() {
  { // ciclo correcto
    Sim s; s.frames(Clase::VACIO, 5); CHECK(s.f.estado() == Estado::ESPERANDO && !s.f.relayOn());
    s.frames(Clase::SOLD_OK, 3); CHECK(s.f.estado() == Estado::DESBLOQUEADO && s.f.relayOn());
    s.frames(Clase::VACIO, 3);   CHECK(s.f.estado() == Estado::EN_PRENSA && s.f.relayOn());
    s.frames(Clase::TAPA_OK, 3); CHECK(s.f.estado() == Estado::OK && !s.f.relayOn() && s.f.contadorOK() == 1);
    s.frames(Clase::VACIO, 3);   CHECK(s.f.estado() == Estado::ESPERANDO);
  }
  { // sin soldadura: NG y relé OFF
    Sim s; s.frames(Clase::SIN_SOLD, 3);
    CHECK(s.f.estado() == Estado::NG && !s.f.relayOn() && s.f.contadorNG() == 1 && s.f.motivo() == MotivoNG::SIN_SOLDADURA);
    s.frames(Clase::SOLD_OK, 10); CHECK(s.f.estado() == Estado::NG);          // no sale solo
    s.f.reinicioSupervisor(); CHECK(s.f.estado() == Estado::ESPERANDO && s.f.exigeRetirarPieza());
    s.frames(Clase::SIN_SOLD, 5); CHECK(s.f.estado() == Estado::ESPERANDO);   // pieza mala sigue ahí: exige retirar
    s.frames(Clase::VACIO, 3); s.frames(Clase::SOLD_OK, 3); CHECK(s.f.estado() == Estado::DESBLOQUEADO);
  }
  { // tapa mal / sin tapa al regresar
    Sim s; s.frames(Clase::SOLD_OK, 3); s.frames(Clase::VACIO, 3); s.frames(Clase::TAPA_MAL, 3);
    CHECK(s.f.estado() == Estado::NG && s.f.motivo() == MotivoNG::TAPA_MAL && !s.f.relayOn());
    Sim u; u.frames(Clase::SOLD_OK, 3); u.frames(Clase::VACIO, 3); u.frames(Clase::SOLD_OK, 3);   // regresó sin tapa
    CHECK(u.f.estado() == Estado::NG && u.f.motivo() == MotivoNG::TAPA_MAL);
  }
  { // la pieza no regresa: relé se apaga a T_DESBLOQ y NG a T_RETORNO
    Config c; c.tDesbloqMs = 5000; c.tRetornoMs = 12000;
    Sim s(c); s.frames(Clase::SOLD_OK, 3); s.frames(Clase::VACIO, 3);
    CHECK(s.f.relayOn()); s.espera(5200); CHECK(!s.f.relayOn() && s.f.estado() == Estado::EN_PRENSA);
    s.espera(8000); CHECK(s.f.estado() == Estado::NG && s.f.motivo() == MotivoNG::NO_REGRESO);
  }
  { // vence el desbloqueo con la pieza aún en la cámara: se bloquea y exige retirarla
    Config c; c.tDesbloqMs = 3000; Sim s(c);
    s.frames(Clase::SOLD_OK, 3); CHECK(s.f.relayOn()); s.espera(3500);
    CHECK(!s.f.relayOn() && s.f.estado() == Estado::ESPERANDO && s.f.exigeRetirarPieza());
    s.frames(Clase::SOLD_OK, 5); CHECK(!s.f.relayOn() && s.f.estado() == Estado::ESPERANDO);   // no se re-desbloquea solo
    s.frames(Clase::VACIO, 3); s.frames(Clase::SOLD_OK, 3); CHECK(s.f.relayOn());
  }
  { // anti-parpadeo y baja confianza
    Sim s; s.frames(Clase::SIN_SOLD, 1); s.frames(Clase::SOLD_OK, 2); s.frames(Clase::SIN_SOLD, 1); s.frames(Clase::SOLD_OK, 2);
    CHECK(s.f.estado() == Estado::ESPERANDO && !s.f.relayOn());                // nunca 3 iguales seguidos
    s.frames(Clase::SOLD_OK, 10, 0.5f); CHECK(!s.f.relayOn());                 // confianza baja no desbloquea
    s.frames(Clase::SOLD_OK, 3, 0.9f);  CHECK(s.f.relayOn());
  }
  { // cambio de pieza durante el desbloqueo: soldadura mala -> NG y bloqueo
    Sim s; s.frames(Clase::SOLD_OK, 3); s.frames(Clase::SIN_SOLD, 3);
    CHECK(s.f.estado() == Estado::NG && !s.f.relayOn() && s.f.motivo() == MotivoNG::SOLDADURA_CAMBIO);
  }
  { // el relé jamás está ON fuera de DESBLOQUEADO / EN_PRENSA
    Sim s; Clase seq[] = {Clase::VACIO, Clase::SOLD_OK, Clase::VACIO, Clase::TAPA_OK, Clase::VACIO, Clase::SIN_SOLD, Clase::TAPA_MAL, Clase::SOLD_OK};
    for (int r = 0; r < 200; r++) { s.frames(seq[std::rand() % 8], 1 + std::rand() % 4, (std::rand() % 10) / 10.0f + 0.1f, 200 + std::rand() % 4000);
      if (s.f.relayOn()) CHECK(s.f.estado() == Estado::DESBLOQUEADO || s.f.estado() == Estado::EN_PRENSA); }
  }
  { // desborde del reloj de 32 bits
    Config c; c.tDesbloqMs = 5000; Sim s(c); s.t = 0xFFFFFF00u;
    s.frames(Clase::SOLD_OK, 3); CHECK(s.f.relayOn()); s.espera(6000); CHECK(!s.f.relayOn());
  }
  std::printf(fallos ? "\n%d FALLOS\n" : "\nTODAS LAS PRUEBAS OK\n", fallos);
  return fallos ? 1 : 0;
}
