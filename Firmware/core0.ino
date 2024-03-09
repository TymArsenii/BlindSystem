void core0(void *pvParameters)
{
  for(;;)
  {
    if(enc_last_read==!digitalRead(26))
    {
      enc_last_read=digitalRead(26);
      if(millis()-debounce_timer>=50)
      {
        last_tick_timer=millis();
        soft_endstps[1][1]=false;
        soft_endstps[1][0]=false;
        debounce_timer=millis();
        if(motor('G')==1)
        {
          steps_counter[1]++;
        }
        else if(motor('G')==-1)
        {
          steps_counter[1]--;
        }
        //Serial.println(steps_counter[1]);
      }
    }
  }
}
