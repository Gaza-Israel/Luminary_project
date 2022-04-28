#include "minimizer.h"

#include "Luminaire.h"

namespace Minimizer {
Luminary *L;
float d[N_LUMINARIES] = {0};
float d_[N_LUMINARIES] = {0};
float y[N_LUMINARIES] = {0};
float cost[N_LUMINARIES] = {1, 1};
float rho = 0.1;
bool d_received[N_LUMINARIES][N_LUMINARIES];
bool all_received = false;

void consensus() {
  Minimizer::optimize();
  Serial.println("optmized");
  Minimizer::all_received = false;
  for (int i = 0; i < N_LUMINARIES; i++) {
    for (int j = 0; j < N_LUMINARIES; j++) {
      d_received[i][j] = false;
    }
    d_[i] = 0;
  }
  Serial.println("reseted");
  char buff[50];
  int ff = 0;
  while (!all_received) {
    I2C_message_protocol::broadcast_local_consensus_dc(d);
    Serial.println(ff++);
    if (I2C::buffer_not_empty()) {
      I2C_message_protocol::parse_message(I2C::pop_message_from_buffer());
      int count = 0;
      for (int i = 0; i < N_LUMINARIES; i++) {
        for (int j = 0; j < N_LUMINARIES; j++) {
          count += Minimizer::d_received[i][j];
        }
      }
      if (count == N_LUMINARIES * N_LUMINARIES) {
        Minimizer::all_received = true;
      }
    }
  }
  update_lagrangian();
  int count = 0;
  for (int i = 0; i < N_LUMINARIES; i++) {
    snprintf(buff, 50, "d%d |  %.5f  %.5f  |", I2C_message_protocol::nodes_addr[i], Minimizer::d[i], Minimizer::d_[i]);
    Serial.println(buff);
    if (abs(Minimizer::d[i] - Minimizer::d_[i]) < 0.001) {
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
      d_int[i] = d_[i] - y[i] / rho - cost[i] / rho;
    } else {
      d_int[i] = d_[i] - y[i] / rho;
    }
  }
  if (is_feasible(d_int)) {
    for (int i = 0; i < N_LUMINARIES; i++) {
      d[i] = d_int[i];
    }
    return;
  }
  float z[N_LUMINARIES];
  float kz = 0;
  float norm_sqr_k = 0;
  float d_borda[5][N_LUMINARIES];
  float c[5] = {0, 0, 0, 0, 0};

  for (int i = 0; i < N_LUMINARIES; i++) {
    z[i] = rho * d_[i] - cost[i] - y[i];
    kz += L->sim._K[i] * z[i];
    norm_sqr_k += L->sim._K[i] * L->sim._K[i];
  }

  for (int i = 0; i < N_LUMINARIES; i++) {
    d_borda[1][i] = z[i] / rho - L->sim._K[i] / norm_sqr_k * (L->get_ext_ilu() - L->contr.get_ref() + kz / rho);
    if (idx == i) {
      d_borda[2][i] = 0;
      d_borda[3][i] = 100;
      d_borda[4][i] = 0;
      d_borda[5][i] = 100;
    } else {
      d_borda[2][i] = z[i] / rho;
      d_borda[3][i] = z[i] / rho;
      d_borda[4][i] = z[i] / rho - L->sim._K[i] / (norm_sqr_k - L->sim._K[i] * L->sim._K[i]) * (L->get_ext_ilu() - L->contr.get_ref() + kz / rho - L->sim._K[i] * z[i] / rho);
      d_borda[5][i] = z[i] / rho - (L->sim._K[i] * (L->get_ext_ilu() - L->contr.get_ref()) + 100 * L->sim._K[i] * L->sim._K[idx]) / (norm_sqr_k - L->sim._K[i] * L->sim._K[i]);
      d_borda[5][i] += L->sim._K[i] / (rho * (norm_sqr_k - L->sim._K[i] * L->sim._K[i])) * (-kz + L->sim._K[i] * z[i]);
    }
    c[0] += d_borda[1][i] * cost[i];
    c[1] += d_borda[2][i] * cost[i];
    c[2] += d_borda[3][i] * cost[i];
    c[3] += d_borda[4][i] * cost[i];
    c[4] += d_borda[5][i] * cost[i];
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
      Minimizer::d[i] = 100;
      Serial.println("No option feasible");
    } else {
      d[i] = d_borda[min_idx][i];
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
    y[i] = y[i] + rho * (d[i] - d_[i]);
  }
}
}  // namespace Minimizer