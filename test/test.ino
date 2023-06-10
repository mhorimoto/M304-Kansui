#include <M304.h>
LCDd lcdd(RS,RW,ENA,DB0,DB1,DB2,DB3,DB4,DB5,DB6,DB7);
EthernetUDP UDP16520;
IPAddress broadcastIP;
extern bool debugMsgFlag(int);
char *pgname = "KanTEST V1.3";
int cposx,cposy,cposp;
int cmode=RUN;
int cmenu=NETCONFIG;
int rlyttl[8];
bool cf,fsf=true;
char lbf[81];
uecsM304 UECS_M304;

void setup(void) {
  int w,j;
  m304Init();
  lcdd.begin(20,4);
  if (Ethernet.begin(st_m.mac)==0) {
    lcdd.setLine(0,2,"NO NET MODE");
    lcdd.LineWrite(0,2);
  }
  msgRun1st();
  if (debugMsgFlag(SO_MSG)) {
    Serial.begin(115200);
    Serial.println(st_m.gw);
    Serial.println(st_m.ip);
    Serial.println(st_m.dns);
    Serial.println(st_m.subnet);
    Serial.println(st_m.cidr);
    Serial.println(broadcastIP);
  }
  j=UDP16520.begin(16520);
  Serial.println(j);
  cposp = 0;
  j = sizeof(UECS_M304);
  Serial.print("sizeof UECS_M304 = ");
  Serial.println(j);
  UECS_M304.enable   = true;
  UECS_M304.room     = 1;
  UECS_M304.region   = 1;
  UECS_M304.order    = 1;
  UECS_M304.priority = 29;
  for (j=0;j<20;j++) {
    UECS_M304.ccm_type[j] = 0;
  }
  strcpy(UECS_M304.ccm_type,"cnd.aMC");
  j = digitalRead(SW_SAFE);
  Serial.print("SW_SAFE = ");
  Serial.println(j);
  if (j==LOW) {
    initEEPROM_UECS();
  }
}

int cnt=0;
void loop(void) {
  char ca,line1[21],t[256];
  static char pca;
  char *xmlDT ;
  static int prvsec;
  extern struct KYBDMEM *ptr_crosskey,*getCrossKey(void);
  int x,y,z,id,hr,mi,mx,io,minsec,j;
  tmElements_t tm;

  xmlDT = CCMFMT;
  if (digitalRead(SW_SAFE)==0) {
    lcdd.setLine(0,1,"  EEPROM Operation  ");
    lcdd.LineWrite(0,1);
    opeEEPROM();
  }
  for(x=0;x<256;x++) {
    t[x] = (char)NULL;
  }
  sprintf(t,xmlDT,UECS_M304.ccm_type,UECS_M304.room,UECS_M304.region,UECS_M304.order,UECS_M304.priority,"0",itoa(broadcastIP));
  Serial.println(t);
  j = UDP16520.beginPacket(broadcastIP, 16520);
  UDP16520.write(t);
  j = UDP16520.endPacket();
  delay(1000);
}

void msgRun1st(void) {
  extern int mask2cidr(IPAddress);
  int cidr,i;
  
  debugSerialOut(cmode,i,"Enter msgRun1st()");
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
  Serial.print("hs=");
  Serial.println(Ethernet.maintain());
  //  if (Ethernet.hardwareStatus()!=EthernetW5500) {
    //    lcdd.setLine(0,2,"NIC IS NO W5500");
    //    lcdd.LineWrite(0,2);
    //  } else {
    lcdd.TextWrite(0,0,2,"IP:");
    st_m.gw = Ethernet.gatewayIP();
    st_m.ip = Ethernet.localIP();
    st_m.dns = Ethernet.dnsServerIP();
    st_m.subnet = Ethernet.subnetMask();
    st_m.cidr   = mask2cidr(st_m.subnet);
    for(i=0;i<4;i++) {
      broadcastIP[i] = ~st_m.subnet[i]|st_m.ip[i];
      //    }
    cposx = 3;
    lcdd.IPWrite(0,cposx,2,st_m.ip);
  }
  debugSerialOut(cmode,i,"Exit msgRun1st()");
}

void debugSerialOut(int a,int b,char *c) {
  char t[80];
  extern bool fsf;
  if (debugMsgFlag(SO_MSG)) {
    sprintf(t,"cmode=%d  cmenu=%d  fsf=%d  key=%s",a,b,fsf,c);
    Serial.println(t);
  }
}

char *itoa(IPAddress a) {
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
    } else {
      sprintf(ccm_type,"Irriopr.%d.aMC",k);
    }
    if (k==0) {
      a = 0x100;
    } else {
      a += 0x20;
    }
    Serial.print("a=");
    Serial.println(a,HEX);
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
