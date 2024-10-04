// Define pin connections & motor's steps per revolution
const int dirPin = 13;
const int stepPin = 14;
int stepsPerRevolution = 70;

void setup()
{
	// Declare pins as Outputs
	pinMode(stepPin, OUTPUT);
	pinMode(dirPin, OUTPUT);
}
void loop()
{
	// Set motor direction clockwise
	digitalWrite(dirPin, HIGH);
	for(int x = 0; x < stepsPerRevolution; x++)
	{
		digitalWrite(stepPin, HIGH);
		delayMicroseconds(2000);
		digitalWrite(stepPin, LOW);
		delayMicroseconds(2000);
	}
  delay(1000);

  digitalWrite(dirPin, LOW);
	for(int x = 0; x < stepsPerRevolution; x++)
	{
		digitalWrite(stepPin, HIGH);
		delayMicroseconds(2000);
		digitalWrite(stepPin, LOW);
		delayMicroseconds(2000);
	}
  delay(1000);
  
  // bool kondisi = true;
  // if(kondisi==true){
  //   stepsPerRevolution = 0;
  // }
  // delay(1000);
  // stepsPerRevolution = 200;
}