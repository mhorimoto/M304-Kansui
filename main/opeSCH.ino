void opeSCH(void) {
  int z,id,hr,mi,mx,io;
  extern struct KYBDMEM *ptr_crosskey,*getCrossKey(void);
  if (fsf) {
    fsf = false;
    cposp = 0;
    cposx = 0;
    cposy = 1;
    lcdd.setLine(cposp,0,"Set Timer           ");
    lcdd.setLine(cposp,1,"01 00:00 00:00 00-00");
    lcdd.setLine(cposp,2,"00000000       OK/NG");
    lcdd.PageWrite(cposp);
    lcdd.setCursor(cposx,cposy); // NO NEED break
  }
  if ((ptr_crosskey->kpos & (K_RIGHT | K_LEFT))==(K_RIGHT|K_LEFT)) {
    lcdd.setCursor(0,3);
    lcdd.print("BOTH ON");
    delay(1000);
    ptr_crosskey->kpos = 0;
    lcdd.setCursor(0,3);
    lcdd.print("       ");
  }
  if (ptr_crosskey->kpos & K_RIGHT) {
    cposx++;
    if (cposx>=20) {
      cposx=0;
      cposy++;
      if (cposy>3) cposy=1;
    }
    ptr_crosskey->kpos &= ~K_RIGHT;  // Reset Flag
    if (cposy==1) {
      switch(cposx) {
      case 2:
      case 5:
      case 8:
      case 11:
      case 14:
      case 17:
	cposx++;
      }
    } else if (cposy==2) {
      if (cposx==8) {
	cposx+=7;
      }
    }
  }
  if (ptr_crosskey->kpos & K_LEFT) {
    cposx--;
    if (cposx<0) {
      cposx=19;
      cposy--;
      if (cposy<1) cposy=3;
    }
    ptr_crosskey->kpos &= ~K_LEFT;  // Reset Flag
    if (cposy==1) {
      switch(cposx) {
      case 2:
      case 5:
      case 8:
      case 11:
      case 14:
      case 17:
	cposx--;
      }
    } else if (cposy==2) {
      if (cposx==14) {
	cposx-=7;
      }
    }
  }
  lcdd.setCursor(cposx,cposy);
  if (cposy==1) {
    mi = '0';
    switch(cposx) {
    case 3:  // Hour
    case 9:
      mx = '2';
      break;
    case 4:
    case 10:
      if (lcdd.CharRead(cposp,cposx-1,cposy)=='2') {
	mx = '3';
      } else {
	mx = '9';
      }
      break;
    case 6:  // Minite
    case 12:
    case 15:
    case 18:
      mx = '5';
      break;
    default:
      mx = '9';
    }
    InputDataButtom(cposp,cposx,cposy,K_DIGIT,ptr_crosskey->kpos,mi,mx);
  } else if (cposy==2) {
    if ((cposx>=0)&&(cposx<8)) {
      mi = '0';
      mx = '1';
      InputDataButtom(cposp,cposx,cposy,K_DIGIT,ptr_crosskey->kpos,mi,mx);
    }
  }

  if (ptr_crosskey->kpos & K_ENT) {
    PushEnter(cposp);
  }
  delay(100);
  ptr_crosskey = getCrossKey();
  if ((ptr_crosskey->longf==true)&&(ptr_crosskey->kpos & K_LEFT)) {
    ptr_crosskey->longf= false;
    ptr_crosskey->kpos = 0;
    cmode = RUN;
    fsf = true;
    lcdd.clear();
    cposy = 0;
    lcdd.setLine(cposp,cposy,pgname);
    lcdd.PageWrite(cposp);
  }
}
