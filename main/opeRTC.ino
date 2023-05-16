void opeRTC(void) {
  extern struct KYBDMEM *ptr_crosskey,*getCrossKey(void);
  if (fsf) {
    fsf = false;
    cposp = 0;
    cposx = 0;
    cposy = 0;
    lcdd.initWriteArea(cposp);
    lcdd.setLine(cposp,0,"Set RTC");
    lcdd.setLine(cposp,1,"  via NTP server");
    lcdd.setLine(cposp,3,"Push ENT Key");
    lcdd.PageWrite(cposp);
  }
  if (ptr_crosskey->kpos & K_ENT) {
    PushEnter(cposp);
  }
  delay(100);
  ptr_crosskey = getCrossKey();
  if ((ptr_crosskey->longf==true)&&(ptr_crosskey->kpos & K_LEFT)) {
    ptr_crosskey->longf= false;
    ptr_crosskey->kpos = 0;
    cmode = CMND;
    fsf = true;
    lcdd.clear();
    cposy = 0;
    lcdd.setLine(cposp,cposy,pgname);
    lcdd.PageWrite(cposp);
  }
}
