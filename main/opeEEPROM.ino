// -*- C++ -*-
//  Arduino と外付け AT24C256 の EEPROMを扱うユーティリティプログラム
//


void opeEEPROM(void) {
  extern struct KYBDMEM *ptr_crosskey,*getCrossKey(void);
  extern bool fsf;
  char cha,cmnd[81],lbf[81],line3[]="Exit:LEFT Key push  ";
  char *pgname = "M304jp EEPROM Ver2.00";
  bool reading,doing;
  String s;
  int  cp ;
  if (fsf) {
    fsf = false;
    lcdd.setLine(0,3,"CONSOLE OPERATION   ");
    lcdd.LineWrite(0,3);
    Serial.begin(115200);
    Serial.println(pgname);
    sprintf(lbf,"Arduino EEPROM SIZE 0-%03XH(%d bytes)",EEPROM.length()-1,EEPROM.length());
    Serial.println(lbf);
    Serial.println("AT24LC256 EEPROM SIZE 0-7FFFH(32,768bytes)");
    cha = (char)NULL;
    cp  = 0;
    doing = true;
  }

  while(doing) {
    wdt_reset();
    reading = true;
    cp = 0;
    Serial.print("% ");
    Serial.flush();
    while(reading) {
      wdt_reset();
      ptr_crosskey->longf= false;
      ptr_crosskey->kpos = 0;
      ptr_crosskey = getCrossKey();
      if ((ptr_crosskey->longf==true)&&(ptr_crosskey->kpos & K_LEFT)) {
        ptr_crosskey->longf= false;
        ptr_crosskey->kpos = 0;
        cmode = CMND;
        fsf = true;
        lcdd.clear();
        lcdd.setLine(0,3,line3);
        lcdd.LineWrite(0,3);
        return;
      }
      if (Serial.available() > 0) {
        cha = Serial.read();
        if ((cha>=0x20)&&(cha<=0x7e)) {
          cmnd[cp] = (char)cha;
          Serial.print(cha);
          cp++;
        } else if (cha==0x08) {
          if (cp>0) {
            cmnd[cp] = (char)NULL;
            Serial.write(0x08);
            Serial.print(" ");
            Serial.write(0x08);
            cp--;
          }
        } else if ((cha==0x0a)||(cha==0x0d)) {
          reading = false;
          Serial.println();
          cmnd[cp] = (char)NULL;
          s = String(cmnd);
        }
      }
    }
    if (s.indexOf("dump")==0) {
      cmnd_dump(s,-1);
    } else if (s.indexOf("atdump")==0) {
      cmnd_dump(s,0);
    } else if (s.indexOf("setb")==0) {
      cmnd_setbyte(s,-1);
    } else if (s.indexOf("atsetb")==0) {
      cmnd_setbyte(s,0);
    } else if (s.indexOf("fill")==0) {
      cmnd_fill(s,-1);
    } else if (s.indexOf("atfill")==0) {
      cmnd_fill(s,0);
    } else if (s.indexOf("exit")==0) {
      doing = false;
      cmode = CMND;
      lcdd.setLine(0,3,line3);
      lcdd.LineWrite(0,3);
      return;
    }
  }
}
