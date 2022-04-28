#ifndef minimizer_h
#define minimizer_h

#include "System_config.h"

class Luminary;

namespace Minimizer {

extern Luminary *L;
bool all_received();
void consensus();
void optimize();
bool is_feasible(float *sol);
void update_lagrangian();

}  // namespace Minimizer

#endif
