#include <M304.h>

#if _M304_H_V < 101
#pragma message("Library M304 is old.")
#else
char *pgname = "Kansui Ver0.05";
LCDd lcdd(RS,RW,ENA,DB0,DB1,DB2,DB3,DB4,DB5,DB6,DB7);

int cposx,cposy,cposp;
int cmode=RUN;
int cmenu=0; // NETCONFIG
byte ip[4] = { 192,168,0,177 };

void setup(void) {
  int w;
  m304Init();
  pinMode(7,OUTPUT);
  lcdd.begin(20,4);
  lcdd.initWriteArea(0);
  lcdd.initWriteArea(1);
  lcdd.cursor();
  lcdd.blink();
  lcdd.clear();
  lcdd.setLine(0,0,pgname);
  cposp = 0;
  cposx = 0;
  cposy = 2;
  lcdd.PageWrite(cposp);
  lcdd.setCursor(cposx,cposy);
  if (Ethernet.begin(st_m.mac)==0) {
    lcdd.setLine(0,2,"NO IP MODE");
    lcdd.LineWrite(0,2);
  } else {
    lcdd.TextWrite(0,0,2,"IP:");
    st_m.gw = Ethernet.gatewayIP();
    st_m.ip = Ethernet.localIP();
    st_m.dns = Ethernet.dnsServerIP();
    cposx = 3;
    lcdd.IPWrite(0,cposx,2,st_m.ip);
  }
}


void loop(void) {
  bool cf;
  int z,id,hr,mi,mx,io;
  char ca,line1[21];
  static char pca;
  static int prvsec;
  extern struct KYBDMEM *ptr_crosskey,*getCrossKey(void);
  uint8_t InputDataButtom(int,int,int,int,uint8_t,int mi='0',int mx='9');
  tmElements_t tm;

  switch(cmode) {
  case RUN:
    if (RTC.read(tm)==0) {
      lcdd.setLine(0,1,"NO RTC PLS SETUP    ");
      lcdd.LineWrite(0,1);
    } else {
      if (prvsec!=tm.Second) {
	prvsec = tm.Second;
	snprintf(line1,21,"%d/%02d/%02d  %02d:%02d:%02d",
		 tm.Year+1970,tm.Month,tm.Day,tm.Hour,tm.Minute,tm.Second);
	lcdd.setLine(0,1,line1);
	lcdd.LineWrite(0,1);
      }
    }
    ptr_crosskey = getCrossKey();
    if (ptr_crosskey->longf==true) {
      ptr_crosskey->longf=false;
      ptr_crosskey->kpos=0;
      cmode=CMND1ST;
    }
    break;
//################################################################
  case CMND1ST:
    cmode=CMND;
    lcdd.noBlink();
    lcdd.setLine(0,0,"Choose Menu         ");
    lcdd.setLine(0,2,"UP/DOWN/ENT Key use ");
    lcdd.setLine(0,3,"Exit:LEFT Key push  ");
    cposp = 0;
    cposx = 0;
    cposy = 1;
    lcdd.PageWrite(cposp);
    lcdd.setCursor(cposx,cposy); // NO NEED break
  case CMND:
    ptr_crosskey = getCrossKey();
    if (ptr_crosskey->kpos & K_LEFT) {
      cmode = RUN;
      ptr_crosskey->longf=false;
      ptr_crosskey->kpos=0;
      lcdd.clear();
      lcdd.blink();
      break;
    }
    cf = false;
    if (ptr_crosskey->kpos & K_UP) {
      ptr_crosskey->kpos &= ~K_UP;
      cmenu++;
      if (cmenu>SCHCONFIG) cmenu=NETCONFIG;
      cf = true;
      Serial.begin(115200);
      Serial.println("cmode=CMND,K_UP");
      Serial.print("cmenu=");
      Serial.println(cmenu);
      Serial.end();
      break;
    }
    if (ptr_crosskey->kpos & K_DOWN) {
      ptr_crosskey->kpos &= ~K_DOWN;
      cmenu--;
      if (cmenu<NETCONFIG) cmenu=SCHCONFIG;
      cf = true;
      Serial.begin(115200);
      Serial.println("cmode=CMND,K_DOWN");
      Serial.print("cmenu=");
      Serial.println(cmenu);
      Serial.end();
      break;
    }
    if (ptr_crosskey->kpos & K_ENT) {
      ptr_crosskey->kpos &= ~K_ENT;
      switch(cmenu) {
      case SCHCONFIG:
	cmode = UTIL1ST;
	break;
      default:
	lcdd.clear();
	cmode = RUN;
	break;
      }
      ptr_crosskey->kpos=0;
      Serial.begin(115200);
      Serial.println("cmode=CMND,K_END");
      Serial.print("cmenu=");
      Serial.println(cmenu);
      Serial.end();
      lcdd.clear();
      lcdd.blink();
      return;
    }
    switch(cmenu) {
    case NETCONFIG: // NET CONFIG
      lcdd.setLine(0,1,"  Network config    ");
      break;
    case RTCCONFIG: // RTC CONFIG
      lcdd.setLine(0,1,"  RTC config        ");
      break;
    case SCHCONFIG: // Schedule CONFIG
      lcdd.setLine(0,1,"  SCHEDULE config   ");
      break;
    }
    if (cf) lcdd.LineWrite(0,1);
    break;
//################################################################
  case UTIL1ST:
    cmode = UTIL;
    lcdd.setLine(0,0,"Set Timer           ");
    lcdd.setLine(0,1,"01 00:00 00:00 00-00");
    lcdd.setLine(0,2,"00000000       OK/NG");
    cposp = 0;
    cposx = 0;
    cposy = 1;
    lcdd.PageWrite(cposp);
    lcdd.setCursor(cposx,cposy); // NO NEED break
  case UTIL:
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
    }
    break;
  }
}


void PushEnter(int p) {
  int x,y;
  extern struct KYBDMEM *ptr_crosskey;
  ptr_crosskey->kpos &= ~K_ENT;  // Reset Flag
  Serial.begin(115200);
  Serial.println("+--------------------+");
  for (y=0;y<4;y++) {
    for (x=0;x<20;x++) {
      Serial.print(lcdd.CharRead(p,x,y));
    }
    Serial.println();
  }
  Serial.println("+--------------------+");
  Serial.end();
}


int InputArithA(int p,int x,int y,int w,int min,int max,bool zp) {
  int v,va,i;
  static int pv=-1000;
  va = analogRead(SELECT_VR);
  v = map(va,0,1023,min,max);
  if (pv!=v) {
    lcdd.IntWrite(p,x,y,w,zp,v);
    pv = v;
  }
  return(v);
}

uint8_t InputDataButtom(int p,int x,int y,int k,uint8_t ud,int mi='0',int mx='9') {
  char c;
  extern struct KYBDMEM *ptr_crosskey;
  c = lcdd.CharRead(p,x,y);
  if (ud==K_UP) {
    digitalWrite(7,HIGH);
    ptr_crosskey->kpos &= ~K_UP;  // Reset Flag
    c++;
    //    Serial.begin(115200);
    //    Serial.print("K_UP: ");
    //    Serial.print(c);
    //    Serial.print(" Limit chk: ");
    switch(k) {
    case K_DIGIT:
      if (c<mi) {
        c = mi;
      } else if (c>mx) {
        c = mi;
      }
    }
    //    Serial.println(c);
    //    Serial.end();
    lcdd.CharWrite(p,x,y,c);
    digitalWrite(7,LOW);
  } else if (ud==K_DOWN) {
    digitalWrite(7,HIGH);
    ptr_crosskey->kpos &= ~K_DOWN;  // Reset Flag
    c--;
    //    Serial.begin(115200);
    //    Serial.print("K_DOWN: ");
    //    Serial.print(c);
    //    Serial.print(" Limit chk: ");
    switch(k) {
    case K_DIGIT:
      if (c<mi) {
        c = mx;
      } else if (c>mx) {
        c = mx;
      }
    }
    //    Serial.println(c);
    //    Serial.end();
    lcdd.CharWrite(p,x,y,c);
    digitalWrite(7,LOW);
  }
  return(-1);
}

#endif
