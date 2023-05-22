void opeNET(void) {
  int z,id,hr,mi,mx,io;
  extern struct KYBDMEM *ptr_crosskey,*getCrossKey(void);
  extern bool fsf;
  if (fsf) {
    fsf = false;
    cposp = 1;
    cposx = 0;
    cposy = 1;
    lcdd.setLine(cposp,0,"NET Config          ");
    if (st_m.dhcpflag) {
      lcdd.TextWrite(cposp,14,0,"DHCP");
    } else {
      lcdd.TextWrite(cposp,14,0,"STATIC");
    }
    lcdd.IPWriteWithCidr(cposp,cposx,cposy,st_m.ip,st_m.cidr);
    cposy++;
    cposx=0;
    lcdd.TextWrite(cposp,cposx,cposy," GW:");
    cposx=4;
    lcdd.IPWrite(cposp,cposx,cposy,st_m.gw);
    cposy++;
    cposx=0;
    lcdd.TextWrite(cposp,cposx,cposy,"DNS:");
    cposx=4;
    lcdd.IPWrite(cposp,cposx,cposy,st_m.dns);
    lcdd.PageWrite(cposp);
    lcdd.setCursor(cposx,cposy); // NO NEED break
    cposp = 0;
  }
  ptr_crosskey = getCrossKey();
  if ((ptr_crosskey->longf==true)&&(ptr_crosskey->kpos & K_LEFT)) {
    ptr_crosskey->longf= false;
    ptr_crosskey->kpos = 0;
    cmode = CMND;
    fsf = true;
    lcdd.clear();
    cposy = 0;
    cposp = 0;
    lcdd.setLine(cposp,cposy,pgname);
    lcdd.PageWrite(cposp);
    debugSerialOut(cmode,cmenu,"EXIT opeNET()");
  }
}
