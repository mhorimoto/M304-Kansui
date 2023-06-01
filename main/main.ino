#include <M304.h>

#if _M304_H_V < 108
#pragma message("Library M304 is old.")
#else
char *pgname = "Kansui Ver1.00A";

typedef struct irrM304 {
  byte id,sthr,stmn,edhr,edmn,inmn,dumn,rly[8];
};

irrM304 irr_m;

LCDd lcdd(RS,RW,ENA,DB0,DB1,DB2,DB3,DB4,DB5,DB6,DB7);

int cposx,cposy,cposp;
int cmode=RUN;
int cmenu=NETCONFIG;
int rlyttl[8];

bool cf,fsf=true;
byte ip[4] = { 192,168,0,177 };
char lbf[81];
extern bool debugMsgFlag(int);

void setup(void) {
  int w;
  m304Init();
  lcdd.begin(20,4);
  msgRun1st();
  if (debugMsgFlag(SO_MSG)) {
    Serial.begin(115200);
  }
  for(w=0;w<8;w++) {
    rlyttl[w] = 0;
  }
}


void loop(void) {
  int x,y,z,id,hr,mi,mx,io,minsec;
  char ca,line1[21],t[81];
  static char pca;
  static int prvsec;
  extern struct KYBDMEM *ptr_crosskey,*getCrossKey(void);
  extern void opeSCH(void),opeRTC(void),opeNET(void),opeRUN(int,int);
  uint8_t InputDataButtom(int,int,int,int,uint8_t,int mi='0',int mx='9');
  tmElements_t tm;

  switch(cmode) {
  case RUN:
    if (fsf) {
      msgRun1st();
      fsf = false;
    }
    if (RTC.read(tm)==0) {
      lcdd.setLine(cposp,1,"NO RTC PLS SETUP    ");
      lcdd.LineWrite(cposp,1);
    } else {
      if (prvsec!=tm.Second) {
	prvsec = tm.Second;
	snprintf(line1,21,"%d/%02d/%02d  %02d:%02d:%02d",	
	 tm.Year+1970,tm.Month,tm.Day,tm.Hour,tm.Minute,tm.Second);
	lcdd.setLine(cposp,1,line1);
	lcdd.LineWrite(cposp,1);
	opeRUN(tm.Hour,tm.Minute);
	Serial.println(t);
	minsec = 0;
	for (x=0;x<8;x++) {
	  if (rlyttl[x]>0) {
	    if (minsec==0) minsec = rlyttl[x];
	    if (minsec>rlyttl[x]) {
	      minsec = rlyttl[x];
	    }
	    digitalWrite(RLY1+x,LOW);
	    rlyttl[x]--;
	  } else {
	    digitalWrite(RLY1+x,HIGH);
	  }
	}
	if (minsec>0) {
	  snprintf(line1,21,"REMAINING=%3d",minsec);
	} else {
	  snprintf(line1,21,"ALL BREAK          ");
	}
	lcdd.setLine(cposp,3,line1);
	lcdd.LineWrite(cposp,3);
      }
    }
    ptr_crosskey = getCrossKey();
    if (ptr_crosskey->longf==true) {
      ptr_crosskey->longf=false;
      ptr_crosskey->kpos=0;
      cmode=CMND;
      fsf = true;
    }
    break;
    //################################################################
  case CMND:
    cf = false;
    if (fsf) {
      debugSerialOut(cmode,cmenu,"Begin point CMND with fsf");
      msgCmnd1st();
    }
    ptr_crosskey = getCrossKey();
    if ((ptr_crosskey->longf==true)&&(ptr_crosskey->kpos & K_LEFT)) {
      debugSerialOut(cmode,cmenu,"Begin point CMND with K_LEFT");
      cmode = RUN;
      fsf = true;
      ptr_crosskey->longf=false;
      ptr_crosskey->kpos=0;
    }
    if (ptr_crosskey->kpos & K_UP) {
      ptr_crosskey->kpos &= ~K_UP;
      cmenu++;
      if (cmenu>EEPROMOPE) cmenu=NETCONFIG;
      cf = true;
      debugSerialOut(cmode,cmenu,"K_UP");      
    }
    if (ptr_crosskey->kpos & K_DOWN) {
      ptr_crosskey->kpos &= ~K_DOWN;
      cmenu--;
      if (cmenu<NETCONFIG) cmenu=EEPROMOPE;
      cf = true;
      debugSerialOut(cmode,cmenu,"K_DOWN");      
    }
    if (ptr_crosskey->kpos & K_ENT) {
      ptr_crosskey->kpos &= ~K_ENT;
      fsf   = true;
      switch(cmenu) {
      case NETCONFIG:
        cmode = NETCMND;
        //opeNET();
	break;
      case RTCCONFIG:
        cmode = RTCCMND;
        //opeRTC();
	break;
      case SCHCONFIG:
        cmode = SCHCMND;
        //opeSCH();
	break;
      case EEPROMOPE:
        cmode = EEPROMCMND;
        break;
      default:
	lcdd.clear();
	cmode = CMND;
	break;
      }
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
    case EEPROMOPE: // EEPROM Operation
      lcdd.setLine(0,1,"  EEPROM Operation  ");
      break;
    }
    if (cf) {
      lcdd.LineWrite(0,1);
      cf = false;
    }
    break;
    //################################################################
  case NETCMND:
    opeNET();
    break;
  case RTCCMND:
    opeRTC();
    break;
  case SCHCMND:
    opeSCH();
    break;
  case EEPROMCMND:
    opeEEPROM();
    break;
  }
}


void PushEnter(int p) {
  int x,y;
  extern struct KYBDMEM *ptr_crosskey;
  ptr_crosskey->kpos &= ~K_ENT;  // Reset Flag
  if (!debugMsgFlag(SO_MSG)) {
    Serial.begin(115200);
  }
  Serial.println("+--------------------+");
  for (y=0;y<4;y++) {
    for (x=0;x<20;x++) {
      Serial.print(lcdd.CharRead(p,x,y));
    }
    Serial.println();
  }
  Serial.println("+--------------------+");
  if (!debugMsgFlag(SO_MSG)) {
    Serial.end();
  }
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
    ptr_crosskey->kpos &= ~K_UP;  // Reset Flag
    c++;

    switch(k) {
    case K_DIGIT:
      if (c<mi) {
        c = mi;
      } else if (c>mx) {
        c = mi;
      }
    }
    lcdd.CharWrite(p,x,y,c);
  } else if (ud==K_DOWN) {
    ptr_crosskey->kpos &= ~K_DOWN;  // Reset Flag
    c--;
    switch(k) {
    case K_DIGIT:
      if (c<mi) {
        c = mx;
      } else if (c>mx) {
        c = mx;
      }
    }
    lcdd.CharWrite(p,x,y,c);
  }
  return(-1);
}

void msgRun1st(void) {
  extern int mask2cidr(IPAddress);
  int cidr;
  
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
    st_m.subnet = Ethernet.subnetMask();
    st_m.cidr   = mask2cidr(st_m.subnet);
    cposx = 3;
    lcdd.IPWrite(0,cposx,2,st_m.ip);
  }
}

void msgCmnd1st(void) {
  extern struct KYBDMEM *ptr_crosskey,*getCrossKey(void);
  lcdd.noBlink();
  lcdd.setLine(0,0,"Choose Menu         ");
  lcdd.setLine(0,2,"UP/DOWN/ENT Key use ");
  lcdd.setLine(0,3,"Exit:LEFT Key push  ");
  cposp = 0;
  cposx = 0;
  cposy = 1;
  lcdd.PageWrite(cposp);
  lcdd.setCursor(cposx,cposy); // NO NEED break
  fsf = false;
  cf = true;
  ptr_crosskey->longf=false;
  ptr_crosskey->kpos=0;
}

void debugSerialOut(int a,int b,char *c) {
  char t[80];
  extern bool fsf;
  if (debugMsgFlag(SO_MSG)) {
    sprintf(t,"cmode=%d  cmenu=%d  fsf=%d  key=%s",a,b,fsf,c);
    Serial.println(t);
  }
}

#endif
