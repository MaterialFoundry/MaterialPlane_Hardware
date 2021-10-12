/**
 * sendIR
 * Modulate the IR signal to send data. It sends 24 bits of data, of which 16 bits ID data and 8 bits command data
 * 
 * The 38khz carrier wave is generated using PWM, given by PWM_LOW, a second state is PWM_HIGH, which means that the IR led is continually on.
 * A digital 1 is represented by a PWM_LOW burst for an IR_SHORT length of time, followed by PWM_HIGH for an IR_LONG length of time.
 * A digital 0 is represented by a PWM_LOW burst followed by a PWM_HIGH both for an IR_SHORT length of time
 * 
 * First a start bit is send: PWM_LOW for IR_LONG followed by PWM_HIGH for IR_SHORT
 * This is followed by the 24 bits of data
 * The transmission ends with a stop bit: PWM_LOW for IR_SHORT
 * 
 * After the transmission, the IR_LED is switched on, so the PAJ sensor can reliably track the location
 */
void sendIR(byte command){
  //Create data integer from the 16 bit serial number and 8 bit command
  uint32_t data = (uint32_t)serialNumber<<8 | command;

  //Send start bits
  setPWM(PWM_LOW);
  delayMicroseconds(IR_LONG);

  setPWM(PWM_HIGH);
  delayMicroseconds(IR_SHORT);

  //Send 24 bit data
  for (int i=23; i>=0; i--) {
    if ((data>>i)&1) {
      setPWM(PWM_LOW);
      delayMicroseconds(IR_SHORT);
      setPWM(PWM_HIGH);
      delayMicroseconds(IR_LONG);
    }
    else {
      setPWM(PWM_LOW);
      delayMicroseconds(IR_SHORT);
      setPWM(PWM_HIGH);
      delayMicroseconds(IR_SHORT);
    }
  }
  
  //Stop bit
  setPWM(PWM_LOW);
  delayMicroseconds(IR_SHORT);

  //Set IR led high
  setPWM(PWM_HIGH);
  
  delay(50);
}
