#include "minimizer.h"

#include "Luminaire.h"
template <class To, class From>
std::enable_if_t<
    sizeof(To) == sizeof(From) &&
        std::is_trivially_copyable_v<From> &&
        std::is_trivially_copyable_v<To>,
    To>
// constexpr support needs compiler magic
bit_cast(const From &src) noexcept {
  static_assert(std::is_trivially_constructible_v<To>,
                "This implementation additionally requires destination type to be trivially constructible");

  To dst;
  memcpy(&dst, &src, sizeof(To));
  return dst;
}
namespace Minimizer {
Luminary *L;

bool all_received() {
  for (int i = 0; i < N_LUMINARIES; i++) {
    if (!L->d_received[i]) {
      return false;
    }
  }
  return true;
}

void consensus() {
  char buff[50];
  int ff = 0;
  int count = 0;

  Minimizer::optimize();
  Serial.println("optmized");
  snprintf(buff, 50, "d%d |  %.5f  %.5f  |", I2C_message_protocol::nodes_addr[0], L->d[0], L->d_[0]);
  Serial.println(buff);
  snprintf(buff, 50, "d%d |  %.5f  %.5f  |", I2C_message_protocol::nodes_addr[1], L->d[1], L->d_[1]);
  Serial.println(buff);

  for (int i = 0; i < N_LUMINARIES; i++) {
    L->d_received[i] = false;
    L->d_[i] = 0;
  }
  Serial.println("reseted");
  unsigned long last = micros();
  for (int i = 0; i < N_LUMINARIES; i++) {
    I2C_message_protocol::broadcast_local_consensus_dc(I2C_message_protocol::nodes_addr[i]);
    Serial.println("broadcasted");
  }
  while (!all_received()) {
   if (micros() - last > 500000) {
     last = micros();
     for (int i = 0; i < N_LUMINARIES; i++) {
       I2C_message_protocol::broadcast_local_consensus_dc(I2C_message_protocol::nodes_addr[i]);
       Serial.println("broadcasted");
     }
   }
    while (I2C::buffer_not_empty()) {
      I2C_message_protocol::parse_message(I2C::pop_message_from_buffer());
    }
  }

  update_lagrangian();
  count = 0;
  for (int i = 0; i < N_LUMINARIES; i++) {
    snprintf(buff, 50, "d%d |  %.5f  %.5f  |", I2C_message_protocol::nodes_addr[i], L->d[i], L->d_[i]);
    Serial.println(buff);
    if (abs(L->d[i] - L->d_[i]) < 0.001) {
      count++;
    }
  }
  if (count == N_LUMINARIES) {
    Serial.println("Consensus achieved");
  }
}

void optimize() {
  // interior
  float d_int[N_LUMINARIES];
  int idx = I2C_message_protocol::addr_is_saved(I2C::get_I2C1_address());
  for (int i = 0; i < N_LUMINARIES; i++) {
    if (idx == i) {
      d_int[i] = L->d_[i] - L->y[i] / L->rho - L->cost[i] / L->rho;
    } else {
      d_int[i] = L->d_[i] - L->y[i] / L->rho;
    }
  }
  if (is_feasible(d_int)) {
    for (int i = 0; i < N_LUMINARIES; i++) {
      L->d[i] = d_int[i];
    }
    return;
  }
  float z[N_LUMINARIES];
  float kz = 0;
  float norm_sqr_k = 0;
  float d_borda[5][N_LUMINARIES];
  float c[5] = {0, 0, 0, 0, 0};

  for (int i = 0; i < N_LUMINARIES; i++) {
    z[i] = L->rho * L->d_[i] - L->cost[i] - L->y[i];
    kz += L->sim._K[i] * z[i];
    norm_sqr_k += L->sim._K[i] * L->sim._K[i];
  }

  for (int i = 0; i < N_LUMINARIES; i++) {
    d_borda[1][i] = z[i] / L->rho - L->sim._K[i] / norm_sqr_k * (L->get_ext_ilu() - L->contr.get_ref() + kz / L->rho);
    if (idx == i) {
      d_borda[2][i] = 0;
      d_borda[3][i] = 100;
      d_borda[4][i] = 0;
      d_borda[5][i] = 100;
    } else {
      d_borda[2][i] = z[i] / L->rho;
      d_borda[3][i] = z[i] / L->rho;
      d_borda[4][i] = z[i] / L->rho - L->sim._K[i] / (norm_sqr_k - L->sim._K[i] * L->sim._K[i]) * (L->get_ext_ilu() - L->contr.get_ref() + kz / L->rho - L->sim._K[i] * z[i] / L->rho);
      d_borda[5][i] = z[i] / L->rho - (L->sim._K[i] * (L->get_ext_ilu() - L->contr.get_ref()) + 100 * L->sim._K[i] * L->sim._K[idx]) / (norm_sqr_k - L->sim._K[i] * L->sim._K[i]);
      d_borda[5][i] += L->sim._K[i] / (L->rho * (norm_sqr_k - L->sim._K[i] * L->sim._K[i])) * (-kz + L->sim._K[i] * z[i]);
    }
    c[0] += d_borda[1][i] * L->cost[i];
    c[1] += d_borda[2][i] * L->cost[i];
    c[2] += d_borda[3][i] * L->cost[i];
    c[3] += d_borda[4][i] * L->cost[i];
    c[4] += d_borda[5][i] * L->cost[i];
  }
  int min_idx = -1;
  int feasible[] = {is_feasible(d_borda[0]), is_feasible(d_borda[1]), is_feasible(d_borda[2]), is_feasible(d_borda[3]), is_feasible(d_borda[4])};
  for (int i = 0; i < 5; i++) {
    if (feasible[i] && min_idx == -1) {
      min_idx = i;
    } else if (feasible[i] && c[i] < c[min_idx]) {
      min_idx = i;
    }
  }
  for (int i = 0; i < N_LUMINARIES; i++) {
    if (min_idx == -1) {
      L->d[i] = 100;
      Serial.println("No option feasible");
    } else {
      L->d[i] = d_borda[min_idx][i];
      // Serial.println("found lowest cost");
    }
  }
}

bool is_feasible(float *sol) {
  int idx = I2C_message_protocol::addr_is_saved(I2C::get_I2C1_address());
  if (sol[idx] < 0 || sol[idx] > 100) {
    return false;
  }
  float ilu = 0;
  for (int i = 0; i < N_LUMINARIES; i++) {
    ilu += L->sim._K[i] * sol[i];
  }
  if (ilu < L->get_ext_ilu() - L->contr.get_ref()) {
    return false;
  }
  return true;
}
void update_lagrangian() {
  for (int i = 0; i < N_LUMINARIES; i++) {
    L->y[i] = L->y[i] + L->rho * (L->d[i] - L->d_[i]);
  }
}
}  // namespace Minimizer
