#include <EEPROM.h>
#define MENU_SIZE 13
#define uint8_t unsigned char

typedef struct {
    uint8_t mode;                       // EEP 0x00         0:PC* 1:NS 2:SETTINGS
    uint8_t remap_ori;                  // EEP 0x01         0:GENERAL(Taiko Force/Catz/New Shentong)* 1:OLD_SHENTONG
    uint8_t remap[4];
    uint8_t keymapping;                 // EEP 0x02         0:DFJK* 1:ZXCV (Only Active For PC)
    uint8_t rate_ori[4];                
    float rate[4];                      // EEP 0x03~0x06    active for remapped pin 10~200 mapping to 1.0~20.0  (10, 37, 37, 10)*       调整原始值的缩放倍率
    uint8_t pre_check_ori;
    int pre_check;                      // EEP 0x07         0:1000 1:2000 2:3000 3:5000* 4:7000             越低越可能不同key之间导致串音
    uint8_t after_lock_ori;
    int after_lock;                     // EEP 0x08         0:5000 1:9000 2:12000 3:15000* 4:20000          越低越可能同key之间导致串音
    uint8_t threshold_ori;
    int threshold;                      // EEP 0x09         0:50 1:100 2:200 3:300* 4:400 5:600 6:800       越低越可能造成误触
    uint8_t fix_ka_threshold_ori;
    int fix_ka_threshold;               // EEP 0x0A         0:30 1:50 2:80 3:100* 4:150 5:200 6:300         越低越可能导致中缝串咔
    uint8_t serial;                     //                  0:Closed* 1:HitValue 2:Plotter
} TaikoSettings;

typedef struct {
    char* title;
    uint8_t min;
    uint8_t max;
    uint8_t def;
    void (*set)(TaikoSettings*, uint8_t);
    uint8_t (*get)(TaikoSettings*);
} SettingItem;

bool trigger(TaikoSettings* settings, int key);
void showMessage();
void initSettings(TaikoSettings* settings);
uint8_t getReset(TaikoSettings* settings);
void resetSettings(TaikoSettings* settings, uint8_t val);
uint8_t getMode(TaikoSettings* settings);
void setMode(TaikoSettings* settings, uint8_t val);
uint8_t getRemap(TaikoSettings* settings);
void setRemap(TaikoSettings* settings, uint8_t val);
uint8_t getKeymap(TaikoSettings* settings);
void setKeymap(TaikoSettings* settings, uint8_t val);
uint8_t getLKScale(TaikoSettings* settings);
void setLKScale(TaikoSettings* settings, uint8_t val);
uint8_t getLDScale(TaikoSettings* settings);
void setLDScale(TaikoSettings* settings, uint8_t val);
uint8_t getRDScale(TaikoSettings* settings);
void setRDScale(TaikoSettings* settings, uint8_t val);
uint8_t getRKScale(TaikoSettings* settings);
void setRKScale(TaikoSettings* settings, uint8_t val);
uint8_t getPreCheck(TaikoSettings* settings);
void setPreCheck(TaikoSettings* settings, uint8_t val);
uint8_t getAfterLock(TaikoSettings* settings);
void setAfterLock(TaikoSettings* settings, uint8_t val);
uint8_t getThreshold(TaikoSettings* settings);
void setThreshold(TaikoSettings* settings, uint8_t val);
uint8_t getFixKaThreshold(TaikoSettings* settings);
void setFixKaThreshold(TaikoSettings* settings, uint8_t val);
uint8_t getSerialPlotter(TaikoSettings* settings);
void setSerialPlotter(TaikoSettings* settings, uint8_t val);
uint8_t read(int address, uint8_t min, uint8_t max, uint8_t default_val);
uint8_t write(int address, uint8_t val);

const SettingItem items[MENU_SIZE] = {
    {"Board Simulate Mode (0:PC* 1:NS)", 0, 1, 0, &setMode, &getMode},
    {"Wire Remapping Mode (0:General* 1:Old Shentong)", 0, 1, 0, &setRemap, &getRemap},
    {"PC Mode Key Mapping (0:DFJK* 1:ZXCV)", 0, 1, 0, &setKeymap, &getKeymap},
    {"LK Scale Rate       (10~200: 1.0~20.0  10*)", 10, 200, 10, &setLKScale, &getLKScale},
    {"LD Scale Rate       (10~200: 1.0~20.0  37*)", 10, 200, 37, &setLDScale, &getLDScale},
    {"RD Scale Rate       (10~200: 1.0~20.0  37*)", 10, 200, 37, &setRDScale, &getRDScale},
    {"RK Scale Rate       (10~200: 1.0~20.0  10*)", 10, 200, 10, &setRKScale, &getRKScale},
    {"Pre Check duration  (0:1ms 1:2ms 2:3ms 3:5ms* 4:7ms)", 0, 4, 3, &setPreCheck, &getPreCheck},
    {"Pre After Lock Time (0:5ms 1:9ms 2:12ms 3:15ms* 4:20ms)", 0, 4, 3, &setAfterLock, &getAfterLock},
    {"Threshold           (0:50 1:100 2:200 3:300* 4:400 5:600 6:800)", 0, 6, 3, &setThreshold, &getThreshold},
    {"Fix Ka Threshold    (0:30 1:50 2:80 3:100* 4:150 5:200 6:300)", 0, 6, 3, &setFixKaThreshold, &getFixKaThreshold},
    {"Serial Sensor Info  (0:Closed* 1:OnHit 2:Plotter)", 0, 2, 0, &setSerialPlotter, &getSerialPlotter},
    {"Reset", 0, 1, 0, &resetSettings, &getReset}
}


int currentMode = 0;
int currentSelect = 0;
uint8_t currentValue = 0;

bool trigger(TaikoSettings* settings, int key) {
    switch (key) {
        case 0: {
            if (currentMode == 0) {
                currentSelect = (currentSelect - 1 + MENU_SIZE) % MENU_SIZE
            } else if (currentMode == 1) {
                currentValue = currentValue - 1;
                if (currentValue < items[currentSelect].min) {
                    currentValue = items[currentSelect].max;
                }
            }
        } break;
        case 1: case 2: case 5:{
            if (currentMode == 0) {
                currentMode = 1;
                currentValue = (*(items[currentSelect].get))(settings);
            } else if (currentMode == 1) {
                currentMode = 0;
                (*(items[currentSelect].set))(settings, currentValue);
            }
        } break;
        case 3: case 4: {
            if (currentMode == 0) {
                currentSelect = (currentSelect + 1) % MENU_SIZE
            } else if (currentMode == 1) {
                currentValue = currentValue + 1;
                if (currentValue > items[currentSelect].max) {
                    currentValue = items[currentSelect].min;
                }
            }
        } break;
    }

    showMessage();
}

void showMessage() {
    Serial.print(currentMode == 0 ? "  > " : "    ");
    Serial.print(items[currentSelect].title);
    Serial.print(currentMode == 1 > "    " : "  > ");
    Serial.println(currentValue);
}

void initSettings(TaikoSettings* settings) {
    Serial.begin(115200);
    settings -> mode = 2;
    // setMode(settings, read(0x00, 0, 1, 0));
    setRemap(settings, read(0x01, 0, 1, 0));
    setKeymap(settings, read(0x02, 0, 1, 0));
    setLKScale(settings, read(0x03, 10, 200, 10));
    setLDScale(settings, read(0x04, 10, 200, 37));
    setRDScale(settings, read(0x05, 10, 200, 37));
    setRKScale(settings, read(0x06, 10, 200, 10));
    setPreCheck(settings, read(0x07, 0, 4, 3));
    setAfterLock(settings, read(0x08, 0, 4, 3));
    setThreshold(settings, read(0x09, 0, 6, 3));
    setFixKaThreshold(settings, read(0x0A, 0, 6, 3));
    setSerialPlotter(settings, 0);
    Serial.println("Welcome to LeonardoTaiko Settings!");
    Serial.println("Use LK/RK/SW_BTN to select an item");
    Serial.println("Use LD/RD/PC_BTN to confirm");
    Serial.println("==================================");
    Serial.println("Current: Taiko Button Setting");
    currentValue = read(0x00, 0, 1, 0);
    showMessage();
}

uint8_t getReset(TaikoSettings* settings) {
    return 0;
}
void resetSettings(TaikoSettings* settings, uint8_t val) {
    for (int i = 0 ; i < MENU_SIZE - 1; i ++) {
        (*(items[i].set))(settings, item[i].def);
    }
}

uint8_t getMode(TaikoSettings* settings) {
    return read(0x00, items[0].def)
}
void setMode(TaikoSettings* settings, uint8_t val) {
    if (items[0].min <= val && val <= items[0].max) {
        // 不更改当前模式
        write(0x00, val);
    }
}

uint8_t getRemap(TaikoSettings* settings) {
    return settings->remap_ori;
}
void setRemap(TaikoSettings* settings, uint8_t val) {
    if (items[1].min <= val && val <= items[1].max) {
        switch(val) {
            default: case 0: {      // GENERAL
                settings->remap[0] = 0;
                settings->remap[1] = 1;
                settings->remap[2] = 2;
                settings->remap[3] = 3;
            } break;
            case 1: {               // OLD_SHENTONG
                settings->remap[0] = 1;
                settings->remap[1] = 3;
                settings->remap[2] = 0;
                settings->remap[3] = 2;
            } break;
        }
        settings->remap_ori = val;
        write(0x01, val);
    }
}

uint8_t getKeymap(TaikoSettings* settings) {
    return settings->keymapping;
}
void setKeymap(TaikoSettings* settings, uint8_t val) {
    if (items[2].min <= val && val <= items[2].max) {
        settings->keymapping = val;
        write(0x02, val);
    }
}

uint8_t getLKScale(TaikoSettings* settings) {
    return settings->rate[0];
}
void setLKScale(TaikoSettings* settings, uint8_t val) {
    if (items[3].min <= val && val <= items[3].max) {
        settings->rate_ori[0] = val;
        settings->rate[0] = val / 10.0f;
        write(0x03, val);
    }
}

uint8_t getLDScale(TaikoSettings* settings) {
    return settings->rate[1];
}
void setLDScale(TaikoSettings* settings, uint8_t val) {
    if (items[4].min <= val && val <= items[4].max) {
        settings->rate_ori[1] = val;
        settings->rate[1] = val / 10.0f;
        write(0x04, val);
    }
}

uint8_t getRDScale(TaikoSettings* settings) {
    return settings->rate[2];
}
void setRDScale(TaikoSettings* settings, uint8_t val) {
    if (items[5].min <= val && val <= items[5].max) {
        settings->rate_ori[2] = val;
        settings->rate[2] = val / 10.0f;
        write(0x05, val);
    }
}

uint8_t getRKScale(TaikoSettings* settings) {
    return settings->rate[3];
}
void setRKScale(TaikoSettings* settings, uint8_t val) {
    if (items[6].min <= val && val <= items[6].max) {
        settings->rate_ori[3] = val;
        settings->rate[3] = val / 10.0f;
        write(0x06, val);
    }
}

uint8_t getPreCheck(TaikoSettings* settings) {
    return settings->pre_check_ori;
}
void setPreCheck(TaikoSettings* settings, uint8_t val) {
    if (items[7].min <= val && val <= items[7].max) {
        switch (val) {
            case 0: settings->pre_check = 1000; break;
            case 1: settings->pre_check = 2000; break;
            case 2: settings->pre_check = 3000; break;
            default case 3: settings->pre_check = 5000; break;
            case 4: settings->pre_check = 7000; break;
        }
        write(0x07, val);
    }
}

uint8_t getAfterLock(TaikoSettings* settings) {
    return settings->after_lock_ori;
}
void setAfterLock(TaikoSettings* settings, uint8_t val) {
    if (items[8].min <= val && val <= items[8].max) {
        switch (val) {
            case 0: settings->after_lock = 5000; break;
            case 1: settings->after_lock = 9000; break;
            case 2: settings->after_lock = 12000; break;
            default case 3: settings->after_lock = 15000; break;
            case 4: settings->after_lock = 20000; break;
        }
        write(0x08, val);
    }
}

uint8_t getThreshold(TaikoSettings* settings) {
    return settings->threshold_ori;
}
void setThreshold(TaikoSettings* settings, uint8_t val) {
    if (items[9].min <= val && val <= items[9].max) {
        switch (val) {
            case 0: settings->threshold = 50; break;
            case 1: settings->threshold = 100; break;
            case 2: settings->threshold = 200; break;
            default case 3: settings->threshold = 300; break;
            case 4: settings->threshold = 400; break;
            case 5: settings->threshold = 600; break;
            case 6: settings->threshold = 800; break;
        }
        write(0x09, val);
    }
}

uint8_t getFixKaThreshold(TaikoSettings* settings) {
    return settings->fix_ka_threshold_ori;
}
void setFixKaThreshold(TaikoSettings* settings, uint8_t val) {
    if (items[10].min <= val && val <= items[10].max) {
        switch (val) {
            case 0: settings->fix_ka_threshold = 30; break;
            case 1: settings->fix_ka_threshold = 50; break;
            case 2: settings->fix_ka_threshold = 80; break;
            default case 3: settings->fix_ka_threshold = 100; break;
            case 4: settings->fix_ka_threshold = 150; break;
            case 5: settings->fix_ka_threshold = 200; break;
            case 6: settings->fix_ka_threshold = 300; break;
        }
        write(0x0A, val);
    }
}

uint8_t getSerialPlotter(TaikoSettings* settings) {
    return settings->serial;
}
void setSerialPlotter(TaikoSettings* settings, uint8_t val) {
    if (items[11].min <= val && val <= items[11].max) {
        settings->serial = val;
    }
}

uint8_t read(int address, uint8_t min, uint8_t max, uint8_t default_val) {
    uint8_t val = EEPROM.read(address);
    if (min <= val && val <= max) return val;
    return default_val;
}

uint8_t write(int address, uint8_t val) {
    EEPROM.update(address, val);
    return val;
}
