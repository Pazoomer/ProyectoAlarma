int led = 13;
int reedswitch = 5;
int buzzer = 8;
int val = 0;

void setup() {
  pinMode(led,OUTPUT);
  pinMode(reedswitch, INPUT);
  pinMode (buzzer, OUTPUT);
}

void loop() {
  val = digitalRead(reedswitch);
  if(val == LOW){
    digitalWrite(led, HIGH);
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(led,LOW);
    digitalWrite(buzzer, LOW);
    delay(200);}
  else{
    digitalWrite(led,LOW);
    digitalWrite(buzzer,LOW);
  }
}
