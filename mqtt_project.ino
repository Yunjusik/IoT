#include <ESP8266WiFi.h>
#include "PubSubClient.h"
#include <DHT11.h>  //dht11센서 라이브러리 정의

char ssid[]="CNL2.4GHz";
char password[]="0987654321";
byte server1[]={165,132,45,240}; // mosquitto 서버의 IP 주소
int port = 1883;
DHT11 dht11(4);  //dht11은 4번핀과 연결
WiFiClient client; //arduino가 클라이언트입장

void msgReceived(char *topic, byte *payload, unsigned int uLen){
  //callback용 msgReceived
  //구독자 여러개 등록되어있으면 토픽이름 서로 다름
  char pBuffer[uLen+1]; //null 포함시킴
  int i;
  for (i=0;i<uLen;i++){
      pBuffer[i]=payload[i];
  }
  pBuffer[i]='\0'; //따옴표 하나
  Serial.println(pBuffer); //1,2
  if(pBuffer[0] =='1'){
    digitalWrite(14, HIGH);
  }else if(pBuffer[0] =='2'){
    digitalWrite(14, LOW);
  }
}

//1 과 2를 넘겨서 led를 컨트롤해야됨. 해당부분을 나중에 web에서 구동하게함.


PubSubClient mqttClient(server1, port, msgReceived, client);

void setup() {
  // put your setup code here, to run once:
  pinMode(14,OUTPUT); // 1이면 on, 2면 off, wemos는 13번핀
  
  
  Serial.begin(115200); //1000- > 1초
  delay(10);
  Serial.println();
  Serial.println();
  Serial.println("Connecting to ~");
  Serial.println(ssid);
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED){
     delay(500);// 0.5초 휴식
     Serial.println(",");   
     
  }
  Serial.println(); 
  Serial.println("Wi-Fi Connected.");
  Serial.println(WiFi.localIP());   
  //MQTT 서버 접속해보기
  if (mqttClient.connect("Arduino")){ //arduino client 접속한다는 정보 전송
     Serial.println("MQTT Broker Connected!"); 
     mqttClient.subscribe("led"); // led topic 등록과정(데이터 읽을 구독자등록)
      
     
  }
}

void loop() {
  // put your main code here, to run repeatedly:
   mqttClient.loop();//모스키토 서버와 연결 유지를 위함
   float tmp, hum;
   int err = dht11.read(hum,tmp); //dht11 센서(4번핀)로부터 hum,tmp읽어서 err에 저장
   if(err==0){                 
    char message[64] = "",pTmpBuf[50], pHumBuf[50];//온도,습도에대한 변수공간을 미리 생성
    dtostrf(tmp,5,2,pTmpBuf); // tmp는 float형이므로, 해당 float를 읽어 5자리수중, 소수점 2자리까지만 읽어서 string으로 변환
    dtostrf(hum,5,2,pHumBuf); // 마찬가지로 hum부분도 위와같이 변환
    sprintf(message,"{\"tmp\":%s,\"hum\":%s}",pTmpBuf,pHumBuf); //변환된 pTmp,pHum을 각각 출력하여 message 변수 json으로 담고
    mqttClient.publish("dht11",message);  //클라이언트(아두이노) 입장에서 해당 json 쿼리 publish(발행)
   }
   delay(3000); //3초 딜레이
}
