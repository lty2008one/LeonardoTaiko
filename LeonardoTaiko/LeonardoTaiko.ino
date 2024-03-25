#include "lib/TaikoSettings.h"
#include "lib/HighPassFilter.h"
#include "lib/LowPassFilter.h"
#include "lib/PressKey.h"



char buf[100];

int high_freq = 1000;
int low_freq = 200;
high_pass_filter_t hfilters[4] = {{0}, {0}, {0}, {0}};
low_pass_filter_t lfilters[4] = {{0}, {0}, {0}, {0}};

int fix_ka_threshold = 100;
int pre_check = 5000;           // 1000~3000  越低越串(不同key)，但是触发速度快
int after_lock = 15000;         // 5000~10000 越低越串（同key），但是触发速度快
int threshold[4] = {400, 400, 400, 400};
// int threshold[4] = {100, 100, 100, 100};
// float rate[4] = {3.7, 1.2, 1, 3.7};
float rate[2] = {1, 3.7};  // ka  don
// float rate[4] = {1, 4, 3.7, 1};
bool wait = false;
unsigned long wait_till[4] = {0, 0, 0, 0};
bool trigger = false;
int high_i = 0;
int high_record[4] = {0, 0, 0, 0};
unsigned long trigger_time = 0;

int pc_send = 60, ns_send = 40;
float d[4][4] = {
  {0.0, 0.1, 0.2, 0.2},
  {0.1, 0.0, 0.2, 0.2},
  {0.2, 0.2, 0.0, 0.1},
  {0.2, 0.2, 0.1, 0.0}
};

const uint8_t pin[4] = {A0, A3, A1, A2};          // 按顺序读取的四个传感器 A0=LD  A3=RK  A1=LK  A2=RD
// const int pin_remap[4] = {1, 3, 0, 2};            // 神童鼓old 映射到真正的输出上, 0=LK 1=LD 2=RD 3=RK
const int pin_remap[4] = {0, 1, 2, 3};            // 神童鼓 映射到真正的输出上, 0=LK 1=LD 2=RD 3=RK
// const int pin_remap[4] = {0, 1, 2, 3};            // 猫鼓 映射到真正的输出上, 0=LK 1=LD 2=RD 3=RK
const KeyUnion NS_LEFT_KATSU[4]  = {{Button::ZL, NS_BTN, NS_BTN_DUR, 0, false}, {Button::L, NS_BTN, NS_BTN_DUR, 0, false}, {Hat::UP, NS_HAT, NS_HAT_DUR, 0, false}, {Hat::LEFT, NS_HAT, NS_HAT_DUR, 0, false}};
const KeyUnion NS_LEFT_DON[3]    = {{Button::LCLICK, NS_BTN, NS_BTN_DUR, 0, false}, {Hat::RIGHT, NS_HAT, NS_HAT_DUR, 0, false}, {Hat::DOWN, NS_HAT, NS_HAT_DUR, 0, false}};
const KeyUnion NS_RIGHT_DON[3]   = {{Button::RCLICK, NS_BTN, NS_BTN_DUR, 0, false}, {Button::Y, NS_BTN, NS_BTN_DUR, 0, false}, {Button::B, NS_BTN, NS_BTN_DUR, 0, false}};
const KeyUnion NS_RIGHT_KATSU[4] = {{Button::ZR, NS_BTN, NS_BTN_DUR, 0, false}, {Button::R, NS_BTN, NS_BTN_DUR, 0, false}, {Button::X, NS_BTN, NS_BTN_DUR, 0, false}, {Button::A, NS_BTN, NS_BTN_DUR, 0, false}};
const KeyUnion PC_LEFT_KATSU[1]  = {{'d', PC_BTN, PC_BTN_DUR, 0, false}};
const KeyUnion PC_LEFT_DON[1]    = {{'f', PC_BTN, PC_BTN_DUR, 0, false}};
const KeyUnion PC_RIGHT_DON[1]   = {{'j', PC_BTN, PC_BTN_DUR, 0, false}};
const KeyUnion PC_RIGHT_KATSU[1] = {{'k', PC_BTN, PC_BTN_DUR, 0, false}};
const KeyUnion *KEYS[4] = {NS_LEFT_KATSU, NS_LEFT_DON, NS_RIGHT_DON, NS_RIGHT_KATSU};
const int NS_SIZE[4] = {sizeof(NS_LEFT_KATSU) / sizeof(KeyUnion), sizeof(NS_LEFT_DON) / sizeof(KeyUnion), sizeof(NS_RIGHT_DON) / sizeof(KeyUnion), sizeof(NS_RIGHT_KATSU) / sizeof(KeyUnion)};
const int PC_SIZE[4] = {sizeof(PC_LEFT_KATSU) / sizeof(KeyUnion), sizeof(PC_LEFT_DON) / sizeof(KeyUnion), sizeof(PC_RIGHT_DON) / sizeof(KeyUnion), sizeof(PC_RIGHT_KATSU) / sizeof(KeyUnion)};
const KeyUnion CTRL_BTN_NS = {Button::PLUS, NS_BTN, 100, 0, false};
const KeyUnion CTRL_BTN_PC = {Button::X, NS_BTN, 100, 0, false};

int mode = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  int pc_status = digitalRead(0);
  int ns_status = digitalRead(1);

  // 初始化读取按键电平
  if (ns_status == LOW && pc_status == HIGH) {
    mode = 1;     // 按下NS按键，初始化为NS模式
    EEPROM.write(0, 1);   // 写入EEPROM
  } else if (pc_status == LOW && ns_status == HIGH) {
    mode = 0;     // 按下PC按键，初始化为PC模式
    EEPROM.write(0, 0);   // 写入EEPROM
  } else if (pc_status == LOW && ns_status == LOW) {
    mode = 2;
    Serial.begin(115200);
    Serial.println("Welcome to LeonardoTaiko Settings!");
    Serial.println("Use LK/RK to select an item");
    Serial.println("Use LD/RD to confirm");
    Serial.println("==================================");
    Serial.println("Current: Taiko Button Setting");
  } else {
    // 没有按任何按键，从EEPROM中读取之前的控制状态
    mode = EEPROM.read(0);
  }

  // 初始化开始连接
  if (mode == 1) {
    pushButton(Button::A, 500, 4); // 初始化时按键自动连接到SWITCH
  } else if(mode == 0) {
    Keyboard.begin();              // 初始化启动按键输入
  }

  // 动态计算采样时长
  unsigned long b = micros();
  loop();
  int space = micros() - b;
  float ts = space / 1000000.0f;
  Init_highPass_alpha(&hfilters[0], ts, high_freq);
  Init_highPass_alpha(&hfilters[1], ts, high_freq);
  Init_highPass_alpha(&hfilters[2], ts, high_freq);
  Init_highPass_alpha(&hfilters[3], ts, high_freq);
  Init_lowPass_alpha(&lfilters[0], ts, low_freq);
  Init_lowPass_alpha(&lfilters[1], ts, low_freq);
  Init_lowPass_alpha(&lfilters[2], ts, low_freq);
  Init_lowPass_alpha(&lfilters[3], ts, low_freq);
}

void loop() {
  // put your main code here, to run repeatedly:
  int r[4] = {
    rate[((pin_remap[0] + 1) % 4) / 2] * analogRead(A0),
    rate[((pin_remap[1] + 1) % 4) / 2] * analogRead(A3),
    rate[((pin_remap[2] + 1) % 4) / 2] * analogRead(A1),
    rate[((pin_remap[3] + 1) % 4) / 2] * analogRead(A2)
  };

  unsigned long b = micros();
  bool pass = false;
  bool pressed = false;

  int c_high = 0, c_high_i = 0;
  int f[4] = {0, 0, 0, 0};
  for (int i = 0; i < 4; i++) {
    f[i] = r[i];
    for (int j = 1; j < 4; j++) {
      int check = (i + j) % 4;
      f[i] -= r[check] * d[i][check];
    }
    f[i] = Low_pass_filter(&lfilters[i], pow(High_pass_filter(&hfilters[i], max(f[i], 0)), 2));
    if (wait_till[i] < b && f[i] > threshold[i]) {
      pass = true;
      if (f[i] > c_high) {
        c_high = f[i];
        c_high_i = i;
      }
    }
  }

  if (b - trigger_time < pre_check) {
    // Serial.println("d");
    high_record[0] = max(high_record[0], f[0]);
    high_record[1] = max(high_record[1], f[1]);
    high_record[2] = max(high_record[2], f[2]);
    high_record[3] = max(high_record[3], f[3]);
  } else if (trigger) {
    // Serial.println("p");
    int high=0, high2=0, high2_i=0;
    for (int i = 0; i < 4; i++) {
      if (high < high_record[i]) {
        high2 = high;
        high2_i = high_i;
        high = high_record[i];
        high_i = i;
      } else if (high2 < high_record[i]) {
        high2 = high_record[i];
        high2_i = i;
      }
      high_record[i] = 0;
    }
    bool skip = high - high2 < fix_ka_threshold;
    if (skip && (pin_remap[high_i] == 1 || pin_remap[high_i] == 2)) skip = false;
    // if (skip && (pin_remap[high2_i] == 1 || pin_remap[high2_i] == 2)) skip = false, high_i = high2_i;

    if (!skip) {
      if (mode == 0) {      // PC模式输出
        switch (pin_remap[high_i]) {
          case 0: pressed = press(PC_SIZE[0], PC_LEFT_KATSU); break;
          case 1: pressed = press(PC_SIZE[1], PC_LEFT_DON); break;
          case 2: pressed = press(PC_SIZE[2], PC_RIGHT_DON); break;
          case 3: pressed = press(PC_SIZE[3], PC_RIGHT_KATSU); break;
        }
      } else {              // NS模式输出
        switch (pin_remap[high_i]) {
          case 0: pressed = press(NS_SIZE[0], NS_LEFT_KATSU); break;
          case 1: pressed = press(NS_SIZE[1], NS_LEFT_DON); break;
          case 2: pressed = press(NS_SIZE[2], NS_RIGHT_DON); break;
          case 3: pressed = press(NS_SIZE[3], NS_RIGHT_KATSU); break;
        }
      }
      if (pressed) {
        // sprintf(buf, "%d %d %d %d", high_i, high, high2_i, high2);
        // Serial.println(buf);
      }
      wait = true;
      wait_till[high_i] = b + after_lock;
    }
    trigger = false;
  } else if (wait) {
    if (f[high_i] < threshold[high_i]) {
      // Serial.println("r");
      wait = false;
    }
  } else if (pass) {
    // Serial.println("t");
    // if (wait_till[c_high_i] < b) {
      trigger = true;
      trigger_time = b;
      high_record[0] = f[0];
      high_record[1] = f[1];
      high_record[2] = f[2];
      high_record[3] = f[3];
    // }
  }

  if (!pressed && !release()) {
    extendKey(b);
  }

  unsigned long used = micros() - b;
  if (used < 180) delayMicroseconds(used);

  // 总负载 184us
  // if (high > 5) {
    // sprintf(buf, "%d %d %d %d %d %d %d %d %d", mode, f[0], f[1], f[2], f[3], r[0], r[1], r[2], r[3]);
    // Serial.println(buf);
  // }

}

unsigned long last_extend = 0;
void extendKey(unsigned long current) {
  if (mode == 1) {
    if (current - last_extend > 500000) {
      int pc = digitalRead(0), ns = digitalRead(1);
      if (ns == LOW) press0(&CTRL_BTN_NS);
      else if (pc == LOW) press0(&CTRL_BTN_PC);
    }
  }
}
