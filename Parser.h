
#ifndef Parser_h
#define Parser_h
#include <Arduino.h>
#include <CommandParser.h>

typedef CommandParser<22, 3, 3, 15, 64> MyCommandParser;

class Luminary;

bool main_timer_callback(struct repeating_timer *t);
void init_globals(Luminary *lum);

class Parser {
 public:
  Parser();
  void setup();

  MyCommandParser _parser;

 private:
  static void set_dc(MyCommandParser::Argument *args, char *response);
  static void get_dc(MyCommandParser::Argument *args, char *response);
  static void set_lux_ref(MyCommandParser::Argument *args, char *response);
  static void get_lux_ref(MyCommandParser::Argument *args, char *response);
  static void get_lux_meas(MyCommandParser::Argument *args, char *response);
  static void set_occ_status(MyCommandParser::Argument *args, char *response);
  static void get_occ_status(MyCommandParser::Argument *args, char *response);
  static void set_anti_windup_status(MyCommandParser::Argument *args, char *response);
  static void get_anti_windup_status(MyCommandParser::Argument *args, char *response);
  static void set_ff_status(MyCommandParser::Argument *args, char *response);
  static void get_ff_status(MyCommandParser::Argument *args, char *response);
  static void set_fb_status(MyCommandParser::Argument *args, char *response);
  static void get_fb_status(MyCommandParser::Argument *args, char *response);
  static void get_ext_ilu(MyCommandParser::Argument *args, char *response);
  static void get_curr_pwr(MyCommandParser::Argument *args, char *response);
  static void get_time(MyCommandParser::Argument *args, char *response);
  static void toggle_stream(MyCommandParser::Argument *args, char *response);
  static void get_hist(MyCommandParser::Argument *args, char *response);
  static void get_acc_enrgy_comsumption(MyCommandParser::Argument *args, char *response);
  static void get_acc_visibility_err(MyCommandParser::Argument *args, char *response);
  static void get_acc_flicker(MyCommandParser::Argument *args, char *response);
  static void set_secundary_loop(MyCommandParser::Argument *args, char *response);
  static void set_controller_gains(MyCommandParser::Argument *args, char *response);
};
#endif
