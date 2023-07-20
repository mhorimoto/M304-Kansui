unsigned long UECSnowmillis;
unsigned long UECSlastmillis;

void UECSloop(void) {
  unsigned long td;
  // 1. udp16520Receive
  // 2. udp16529Receive and Send
  // 3. udp16521Receive and Send
  // << USER MANAGEMENT >>
  // 4. udp16520Send
  UECSupdate16520portReceive(&UECStempCCM, UECSnowmillis);
  UECSupdate16529port(&UECStempCCM);
  UECSupdate16521port(&UECStempCCM);
  UserEveryLoop();  

  UECSnowmillis = millis();
  if (UECSnowmillis==UECSlastmillis) {
    return;
  }
 
  //how long elapsed?
  td=UECSnowmillis-UECSlastmillis;
  //check overflow 
  if ( UECSnowmillis<UECSlastmillis ) {
    td = (4294967295-UECSlastmillis) + UECSnowmillis;
  }

  //over 1msec
  UECSsyscounter1s+=td;
  UECSlastmillis=UECSnowmillis;
  UECSautomaticValidManager(td);

  if ( UECSsyscounter1s < 999 ) {
    return;
  }
  //over 1000msec
  UECSsyscounter1s = 0;
  UECSsyscounter60s++;
    
  if(UECSsyscounter60s >= 60) {
    UserEveryMinute();
    UECSsyscounter60s = 0;  
  }
    
    UECSautomaticSendManager();
    UserEverySecond();
    
    for(int i = 0; i < U_MAX_CCM; i++)
      {
     if(U_ccmList[i].sender && U_ccmList[i].flagStimeRfirst && U_ccmList[i].ccmLevel != NONE)
     	{
        UECSCreateCCMPacketAndSend(&U_ccmList[i]);
       }

    }   

}
