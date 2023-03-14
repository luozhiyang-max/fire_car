#include <Arduino.h>
#include <ESP8266WiFi.h> // 本程序使用ESP8266WiFi库

const char *ssid = "L";            // 需要连接到的WiFi名
const char *password = "12345678"; // 连接的WiFi密码

WiFiServer server(1234);//TCP Server端口
WiFiClient serverClients;
//电机驱动引脚宏定义
#define left_motor0 16
#define left_motor1 5
#define right_motor0 0
#define right_motor1 2

#define left_en 14  // 左路电机调速PA15
#define right_en 12 // 右路电机调速PB3

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
  Serial.begin(115200); // 初始化串口通讯波特率为115200
                        // pinMode(2, OUTPUT);//8266 01sLED引脚
                        // digitalWrite(2,HIGH);

  user_setup();
  WiFi.mode(WIFI_STA);                // 设置Wifi工作模式为STA,默认为AP+STA模式
  WiFi.begin(ssid, password);         // 通过wifi名和密码连接到Wifi
  Serial.print("\r\nConnecting to "); // 串口监视器输出网络连接信息
  Serial.print(ssid);
  Serial.println(" ..."); // 显示NodeMCU正在尝试WiFi连接

  int i = 0;                            // 检查WiFi是否连接成功
  while (WiFi.status() != WL_CONNECTED) // WiFi.status()函数的返回值是由NodeMCU的WiFi连接状态所决定的。
  {                                     // 如果WiFi连接成功则返回值为WL_CONNECTED
    delay(1000);                        // 此处通过While循环让NodeMCU每隔一秒钟检查一次WiFi.status()函数返回值
    Serial.print("waiting for ");
    Serial.print(i++);
    Serial.println("s...");
  }

  Serial.println("");                // WiFi连接成功后
  Serial.println("WiFi connected!"); // NodeMCU将通过串口监视器输出"连接成功"信息。
  Serial.print("IP address: ");      // 同时还将输出NodeMCU的IP地址。这一功能是通过调用
  Serial.println(WiFi.localIP());    // WiFi.localIP()函数来实现的。该函数的返回值即NodeMCU的IP地址。

  /* 开启TCPServer */
  server.begin();
  server.setNoDelay(true);
  while (!server.hasClient())
  {
    Serial.println("No Connection");
    delay(5000);
  }
  serverClients = server.available();
  digitalWrite(2,LOW);//有Clinet连接上后板载LED点亮

}

void loop()
{
  /* 等待TCP服务器返回消息 */
  if (serverClients.available())
  {
    tcp_read_char();
  }
  /* 向TCP服务器返回串口收到的消息 */
  if (Serial.available())
  {
    String rx_data = serial_read();
    serverClients.print(rx_data);
  }

}

/**
 * @brief 处理TCP服务器下发的指令信息并转发给STM32
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
    if (TCP_data[count - 1] == '\n' && TCP_data[count - 2] == '\r')//检测数据包尾是否正确
    {
      count = 0;
      if (TCP_data[0] == 'X')   //检测是否为移动底盘的数据包头
      {
        sscanf(TCP_data, "%c:%d,%c:%d", &a, &y, &b, &x);
        chassis_move(x, (int)(y / 2));
      }
      else                    //非移动底盘的数据直接转发给STM32
      {
        Serial.print(TCP_data);
      }
      memset(TCP_data, 0, sizeof(TCP_data));//清空数组
    }
  }
}

/**
 * @brief 读取串口信息
 * 
 * @return String 返回串口信息
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
 * @brief 电机驱动引脚初始化
 * 
 */
void user_setup()
{
  // 电机输出引脚
  pinMode(left_motor0, OUTPUT);
  pinMode(left_motor1, OUTPUT);
  pinMode(right_motor0, OUTPUT);
  pinMode(right_motor1, OUTPUT);
  // PWM输出引脚
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
 * @brief 电机驱动函数
 * 
 * @param left 左路电机速度
 * @param right 右路电机速度
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
 * @brief 底盘运动函数
 * 
 * @param vx 前进速度
 * @param vy 转弯速度
 */
void chassis_move(int vx, int vy)
{
  motor_move(vx + vy, vx - vy);
}