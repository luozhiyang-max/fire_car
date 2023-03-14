#include <Arduino.h>
#include <ESP8266WiFi.h> // ������ʹ��ESP8266WiFi��

const char *ssid = "L";            // ��Ҫ���ӵ���WiFi��
const char *password = "12345678"; // ���ӵ�WiFi����

WiFiServer server(1234);//TCP Server�˿�
WiFiClient serverClients;
//����������ź궨��
#define left_motor0 16
#define left_motor1 5
#define right_motor0 0
#define right_motor1 2

#define left_en 14  // ��·�������PA15
#define right_en 12 // ��·�������PB3

String serial_read();
String tcp_read();

char TCP_data[20] = {};
char Serial_data[20] = {};

void tcp_read_char();
void serial_read_char();
void user_setup();
void motor_move(int left, int right);
void chassis_move(int vx, int vy);

void setup()
{
  Serial.begin(115200); // ��ʼ������ͨѶ������Ϊ115200
                        // pinMode(2, OUTPUT);//8266 01sLED����
                        // digitalWrite(2,HIGH);

  user_setup();
  WiFi.mode(WIFI_STA);                // ����Wifi����ģʽΪSTA,Ĭ��ΪAP+STAģʽ
  WiFi.begin(ssid, password);         // ͨ��wifi�����������ӵ�Wifi
  Serial.print("\r\nConnecting to "); // ���ڼ������������������Ϣ
  Serial.print(ssid);
  Serial.println(" ..."); // ��ʾNodeMCU���ڳ���WiFi����

  int i = 0;                            // ���WiFi�Ƿ����ӳɹ�
  while (WiFi.status() != WL_CONNECTED) // WiFi.status()�����ķ���ֵ����NodeMCU��WiFi����״̬�������ġ�
  {                                     // ���WiFi���ӳɹ��򷵻�ֵΪWL_CONNECTED
    delay(1000);                        // �˴�ͨ��Whileѭ����NodeMCUÿ��һ���Ӽ��һ��WiFi.status()��������ֵ
    Serial.print("waiting for ");
    Serial.print(i++);
    Serial.println("s...");
  }

  Serial.println("");                // WiFi���ӳɹ���
  Serial.println("WiFi connected!"); // NodeMCU��ͨ�����ڼ��������"���ӳɹ�"��Ϣ��
  Serial.print("IP address: ");      // ͬʱ�������NodeMCU��IP��ַ����һ������ͨ������
  Serial.println(WiFi.localIP());    // WiFi.localIP()������ʵ�ֵġ��ú����ķ���ֵ��NodeMCU��IP��ַ��

  /* ����TCPServer */
  server.begin();
  server.setNoDelay(true);
  while (!server.hasClient())
  {
    Serial.println("No Connection");
    delay(5000);
  }
  serverClients = server.available();
  digitalWrite(2,LOW);//��Clinet�����Ϻ����LED����

}

void loop()
{
  /* �ȴ�TCP������������Ϣ */
  if (serverClients.available())
  {
    tcp_read_char();
  }
  /* ��TCP���������ش����յ�����Ϣ */
  if (Serial.available())
  {
    String rx_data = serial_read();
    serverClients.print(rx_data);
  }

}

/**
 * @brief ����TCP�������·���ָ����Ϣ��ת����STM32
 * 
 */
void tcp_read_char()
{
  uint8_t count = 0;
  int x = 0, y = 0;
  char a, b;
  while (serverClients.available())
  {
    TCP_data[count++] = static_cast<char>(serverClients.read());
    if (TCP_data[count - 1] == '\n' && TCP_data[count - 2] == '\r')//������ݰ�β�Ƿ���ȷ
    {
      count = 0;
      if (TCP_data[0] == 'X')   //����Ƿ�Ϊ�ƶ����̵����ݰ�ͷ
      {
        sscanf(TCP_data, "%c:%d,%c:%d", &a, &y, &b, &x);
        chassis_move(x, (int)(y / 2));
      }
      else                    //���ƶ����̵�����ֱ��ת����STM32
      {
        Serial.print(TCP_data);
      }
      memset(TCP_data, 0, sizeof(TCP_data));//�������
    }
  }
}

/**
 * @brief ��ȡ������Ϣ
 * 
 * @return String ���ش�����Ϣ
 */
String serial_read()
{
  String rx_buf;
  char data;
  while (Serial.available())
  {
    data = static_cast<char>(Serial.read());
    rx_buf = rx_buf + data;
  }
  return rx_buf;
}
/**
 * @brief ����������ų�ʼ��
 * 
 */
void user_setup()
{
  // ����������
  pinMode(left_motor0, OUTPUT);
  pinMode(left_motor1, OUTPUT);
  pinMode(right_motor0, OUTPUT);
  pinMode(right_motor1, OUTPUT);
  // PWM�������
  pinMode(left_en, OUTPUT);
  pinMode(right_en, OUTPUT);
  pinMode(13, OUTPUT);

  digitalWrite(left_motor0, LOW);
  digitalWrite(left_motor1, LOW);
  digitalWrite(right_motor0, LOW);
  digitalWrite(right_motor1, LOW);
  digitalWrite(left_en, LOW);
  digitalWrite(right_en, LOW);
}
/**
 * @brief �����������
 * 
 * @param left ��·����ٶ�
 * @param right ��·����ٶ�
 */
void motor_move(int left, int right)
{
  if (left > 0)
  {
    digitalWrite(left_motor1, HIGH);
    digitalWrite(left_motor0, LOW);
  }
  else
  {
    digitalWrite(left_motor1, LOW);
    digitalWrite(left_motor0, HIGH);
  }
  if (right > 0)
  {
    digitalWrite(right_motor1, HIGH);
    digitalWrite(right_motor0, LOW);
  }
  else
  {
    digitalWrite(right_motor1, LOW);
    digitalWrite(right_motor0, HIGH);
  }
  left = left > 0 ? left : (-left);
  right = right > 0 ? right : (-right);
  analogWrite(left_en, (int)(left));
  analogWrite(right_en, (int)(right));
}
/**
 * @brief �����˶�����
 * 
 * @param vx ǰ���ٶ�
 * @param vy ת���ٶ�
 */
void chassis_move(int vx, int vy)
{
  motor_move(vx + vy, vx - vy);
}