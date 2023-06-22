#include <M304.h>
#include <avr/wdt.h>

#if _M304_H_V < 110
#pragma message("Library M304 is old.")
#else
uint8_t mcusr_mirror __attribute__ ((section (".noinit")));
void get_mcusr(void)     \
  __attribute__((naked)) \
  __attribute__((section(".init3")));
void get_mcusr(void) {
  mcusr_mirror = MCUSR;
  MCUSR = 0;
  wdt_disable();
}

char *pgname = "Kansui Ver1.32";

typedef struct irrM304 {
  byte id,sthr,stmn,edhr,edmn,inmn,dumn,rly[8];
};

irrM304 irr_m;

LCDd lcdd(RS,RW,ENA,DB0,DB1,DB2,DB3,DB4,DB5,DB6,DB7);
EthernetUDP UDP16520;
//EthernetUDP UECS_UDP16529;
//EthernetUDP UECS_UDP16521;
//EthernetServer UECSlogserver(80);
//EthernetClient UECSclient;
IPAddress broadcastIP;

int cposx,cposy,cposp;
int cmode=RUN;
int cmenu=NETCONFIG;
int rlyttl[8];

bool cf,fsf=true;
byte ip[4] = { 192,168,0,177 };
char lbf[81];
extern bool debugMsgFlag(int);


void setup(void) {
  int w,j;
  char ccm_type[21];
  m304Init();
  lcdd.begin(20,4);
  if (Ethernet.begin(st_m.mac)==0) {
    lcdd.setLine(0,2,"NO NET MODE");
    lcdd.LineWrite(0,2);
  }
  configure_wdt();
  msgRun1st();
  wdt_reset();
  if (debugMsgFlag(SO_MSG)) {
    Serial.begin(115200);
    Serial.println(st_m.gw);
    Serial.println(st_m.ip);
    Serial.println(st_m.dns);
    Serial.println(st_m.subnet);
    Serial.println(st_m.cidr);
    Serial.println(broadcastIP);
  }
  for(w=0;w<8;w++) {
    rlyttl[w] = 0;
  }
  for (j=0;j<20;j++) {
    ccm_type[j] = 0;
  }
  //  strcpy(ccm_type,"cnd.aMC");
  j = digitalRead(SW_SAFE);
  if (j==LOW) {
    for(w=0;w<0x7;w++) {
      if (ccm_type[w]!=atmem.read(w+0x106)) {
        initEEPROM_UECS();
        w = 8;
        break;
      }
    }
  }
  UDP16520.begin(16520);
  for (j=0;j<20;j++) {
    ccm_type[j] = 0;
  }
  strcpy(ccm_type,"cnd.aMC");
  j = digitalRead(SW_SAFE);
  if (j==LOW) {
    for(w=0;w<0x7;w++) {
      if (ccm_type[w]!=atmem.read(w+0x106)) {
        initEEPROM_UECS();
        w = 8;
        break;
      }
    }
  }
  sendUECSpacket(0,"2048"); // setup completed 0x800

}


void loop(void) {
  int x,y,z,id,hr,mi,mx,io,minsec,j;
  char ca,line1[21];
  static char pca;
  static int prvsec;
  extern struct KYBDMEM *ptr_crosskey,*getCrossKey(void);
  extern void opeSCH(void),opeRTC(void),opeNET(void),opeRUN(int,int);
  uint8_t InputDataButtom(int,int,int,int,uint8_t,int mi='0',int mx='9');
  tmElements_t tm;
 
  wdt_reset();
  if (digitalRead(SW_SAFE)==0) {
    lcdd.setLine(0,1,"  EEPROM Operation  ");
    lcdd.LineWrite(0,1);
    opeEEPROM();
  }
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
        //
        sendUECSpacket(0,"0");
        //
      }
    }
    ptr_crosskey = getCrossKey();
    if (ptr_crosskey->longf==true) {
      ptr_crosskey->longf=false;
      ptr_crosskey->kpos=0;
      cmode=CMND;
      fsf = true;
    }
    wdt_reset();
    break;
    //################################################################
  case CMND:
    cf = false;
    if (fsf) {
      debugSerialOut(cmode,cmenu,"Begin point CMND with fsf");
      msgCmnd1st();
    }
    wdt_reset();
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
    wdt_reset();
    opeNET();
    break;
  case RTCCMND:
    wdt_reset();
    opeRTC();
    break;
  case SCHCMND:
    wdt_reset();
    opeSCH();
    break;
  case EEPROMCMND:
    wdt_reset();
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
  int cidr,i;
  
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
  if (Ethernet.maintain()!=0) {
    lcdd.setLine(0,2,"NIC IS NO W5500");
    lcdd.LineWrite(0,2);
  } else {
    lcdd.TextWrite(0,0,2,"IP:");
    st_m.gw = Ethernet.gatewayIP();
    st_m.ip = Ethernet.localIP();
    st_m.dns = Ethernet.dnsServerIP();
    st_m.subnet = Ethernet.subnetMask();
    st_m.cidr   = mask2cidr(st_m.subnet);
    for(i=0;i<4;i++) {
      broadcastIP[i] = ~st_m.subnet[i]|st_m.ip[i];
    }
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

char *itoaddr(IPAddress a) {
  char ia[16],*iap;
  iap = &ia[0];
  sprintf(iap,"%d.%d.%d.%d",a[0],a[1],a[2],a[3]);
  return(iap);
}

void initEEPROM_UECS(void) {
  int w,a,j,k;
  w = 9;      // cnd + RLY1..8
  bool enable;
  byte room,region,priority;
  int  order;
  char ccm_type[20];

  enable = true;
  room   = 1;
  region = 1;
  order  = 1;
  priority = 15;
  for (k=0;k<w;k++) {
    for (j=0;j<20;j++) {
      ccm_type[j] = 0;
    }
    if (k==0) {
      strcpy(ccm_type,"cnd.aMC");
    } else if (k==6) {
      sprintf(ccm_type,"AirHumFogopr.%d",k);
    } else {
      sprintf(ccm_type,"Irriopr.%d",k);
    }
    if (k==0) {
      a = 0x100;
    } else {
      a += 0x20;
    }
    atmem.write(a,enable);
    atmem.write(a+1,room);
    atmem.write(a+2,region);
    atmem.write(a+3,(order&0xff));
    atmem.write(a+4,(order>>8)&0xff);
    atmem.write(a+5,priority);
    for (j=0;j<20;j++) {
      atmem.write(a+6+j,ccm_type[j]);
    }
  }
}

void sendUECSpacket(int id,char *v) {
  char t[256];
  char *xmlDT;
  bool enable;
  byte room,region,priority;
  int  order,x,a,j;
  char ccm_type[20];
  byte ordh,ordl;
  for(x=0;x<256;x++) {
    t[x] = (char)NULL;
  }
  a = 0x100+(id*0x20);
  xmlDT = CCMFMT;
  enable = atmem.read(a);
  if (enable==false) {
    return;
  }
  room = atmem.read(a+1);
  region = atmem.read(a+2);
  ordl = atmem.read(a+3);
  ordh = atmem.read(a+4);
  order = (ordh<<8)+ordl;
  priority = atmem.read(a+5);
  for (j=0;j<20;j++) {
    ccm_type[j] = atmem.read(a+6+j);
  }
  sprintf(t,xmlDT,ccm_type,room,region,
          order,priority,v,itoaddr(st_m.ip));
  
  if (debugMsgFlag(SO_MSG)) {
    Serial.println(t);
  }
  UDP16520.beginPacket(broadcastIP, 16520);
  UDP16520.write(t);
  UDP16520.endPacket();
}

void configure_wdt(void) {
  cli();                           // disable interrupts for changing the registers
  MCUSR = 0;                       // reset status register flags
                                   // Put timer in interrupt-only mode:
  WDTCSR |= 0b00011000;            // Set WDCE (5th from left) and WDE (4th from left) to enter config mode,
                                   // using bitwise OR assignment (leaves other bits unchanged).
  WDTCSR =  0b00001000 | 0b100001; // clr WDIE: interrupt enabled
                                   // set WDE: reset disabled
                                   // and set delay interval (right side of bar) to 8 seconds
  sei();                           // re-enable interrupts
                                   // reminder of the definitions for the time before firing
                                   // delay interval patterns:
                                   //  16 ms:     0b000000
                                   //  500 ms:    0b000101
                                   //  1 second:  0b000110
                                   //  2 seconds: 0b000111
                                   //  4 seconds: 0b100000
                                   //  8 seconds: 0b100001
}

#endif
