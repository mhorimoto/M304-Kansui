#include <M304.h>

void opeRUN(int hr,int mn) {
  int id, a,i,j,k;
  byte s[2];
  int r;
  char t[81];
  for(id=0;id<100;id++) {
    a = LC_SCH_START+(id*0x10);
    if (atmem.read(a)!=0xff) {
      r = timeDecision(id,hr,mn);
      if (r==UNMATCH) continue;
      Serial.begin(115200);
      sprintf(t,"opeRUN id=%d start=%02d:%02d end=%02d:%02d inter=%02d-%02d cur=%02d:%02d r=%d",id,
	      atmem.read(a),atmem.read(a+1),
	      atmem.read(a+2),atmem.read(a+3),
	      atmem.read(a+4),atmem.read(a+5),
	      hr,mn,r);
      Serial.println(t);
      switch(r) {
      case CHANGE_MAKE:
	s[0] = atmem.read(a+14);
	s[1] = atmem.read(a+15);
	for(i=0;i<4;i++) {
	  j = (s[0]>>(i*2))&0x3;
	  k = (s[1]>>(i*2))&0x3;
	  sprintf(t,"opeRUN rly=%02x%02x i=%d j=%d k=%d",s[0],s[1],i,j,k);
	  Serial.println(t);
	  if (j) {
	    digitalWrite(RLY4-i,LOW);
            //	    sprintf(t,"opeRUN rly=%d is MAKE",RLY4-i);
            //	    Serial.println(t);
	  } else {
	    digitalWrite(RLY4-i,HIGH);
            //	    sprintf(t,"opeRUN rly=%d is BREAK",RLY4-i);
            //	    Serial.println(t);
	  }
	  if (k) {
	    digitalWrite(RLY8-i,LOW);
            //	    sprintf(t,"opeRUN rly=%d is MAKE",RLY8-i);
            //	    Serial.println(t);
	  } else {
            //	    sprintf(t,"opeRUN rly=%d is BREAK",RLY8-i);
            //	    Serial.println(t);
	    digitalWrite(RLY8-i,HIGH);
	  }
	}
        break;
      case CHANGE_BREAK:
        for (i=0;i<8;i++) {
          digitalWrite(RLY1+i,HIGH);
        }
        break;
      }
    }
  }
  delay(1000);
}

int timeDecision(int id,int curhr,int curmn) {
  static bool countStart;
  byte d,sthr,stmn,edhr,edmn,inmn,dumn,rly0,rly1;
  int  addr,sttime,edtime,startm,endm,curtim,pmin;
  countStart = false;
  addr = LC_SCH_START+(id*0x10);
  sthr   = (int)atmem.read(addr+STHR);
  stmn   = (int)atmem.read(addr+STMN);
  if (sthr>24) return ERROR;
  sttime = sthr * 100 + stmn;
  edhr   = (int)atmem.read(addr+EDHR);
  edmn   = (int)atmem.read(addr+EDMN);
  if (edhr>24) return ERROR;
  edtime = edhr * 100 + edmn;
  startm = (int)atmem.read(addr+INMN);
  endm   = (int)atmem.read(addr+DUMN);
  curtim = curhr*100+curmn;
  if (startm<2) return ERROR; // 2分毎未満は受け付けない
  if ((sttime<=curtim)&&(curtim<=edtime)) {  // 設定時刻内ならば有効
    if (((curmn % startm)==0)&& !countStart) {  // MAKE分になった
      countStart = true;
      pmin = curmn;
    }
    if (countStart && ((pmin+endm)>curmn)) {  // MAKE分数内
      return CHANGE_MAKE; // 有効を返す
    } else {
      countStart = false;
      return CHANGE_BREAK;
    }
  }
  countStart = false;
  return UNMATCH;
}
  
  
int getHourMin(byte h,byte m) {
  int hm;
  hm = h*100+m;
  return hm;
}

