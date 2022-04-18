#include "cglobal.h"

int cmain(){
  cur_set_attr(0x03);
  if(start_sys()){
    run();
    close_sys();
  }
  return 0;
}
