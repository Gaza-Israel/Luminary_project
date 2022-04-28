#ifndef minimizer_h
#define minimizer_h

#include "System_config.h"

class Luminary;

namespace Minimizer {

extern Luminary *L;
extern float d[N_LUMINARIES];
extern float d_[N_LUMINARIES];
extern float y[N_LUMINARIES];
extern float cost[N_LUMINARIES];
extern float rho;
extern bool d_received[N_LUMINARIES][N_LUMINARIES];
extern bool all_received;

void consensus();
void optimize();
bool is_feasible(float *sol);
void update_lagrangian();

}  // namespace Minimizer

#endif
