#include <Bluepad32.h>

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

#define BUTTON_X  0x0001
#define BUTTON_SQUARE  0x0004

const int DEADZONE = 300;

void onConnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            Serial.printf("Controller connected, index=%d\n", i);
            myControllers[i] = ctl;
            break;
        }
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            Serial.printf("Controller disconnected, index=%d\n", i);
            myControllers[i] = nullptr;
            break;
        }
    }
}

void processGamepad(ControllerPtr ctl) {
    int x = ctl->axisX();  // Izquierda (-512) a derecha (512)
    int y = ctl->axisY();  // Arriba (-512) a abajo (512)

    // Movimiento con joystick
    if (y < -DEADZONE) {
        Serial.write("w");  // Adelante
        Serial.write("\n");
    } else if (y > DEADZONE) {
        Serial.write("s");  // Atrás
        Serial.write("\n");
    }

    if (x < -DEADZONE) {
        Serial.write("a");  // Izquierda
        Serial.write("\n");
    } else if (x > DEADZONE) {
        Serial.write("d");  // Derecha
        Serial.write("\n");
    }

    // Botón X
    if (ctl->buttons() & BUTTON_X) {
        Serial.write("x");
        Serial.write("\n");
    }

    // Botón Cuadrado
    //if (ctl->buttons() & BUTTON_SQUARE) {
    //    Serial.write("");
    //    Serial.write("\n");
    //}
}

void processControllers() {
    for (auto ctl : myControllers) {
        if (ctl && ctl->isConnected() && ctl->hasData()) {
            if (ctl->isGamepad()) {
                processGamepad(ctl);
            }
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    BP32.setup(&onConnectedController, &onDisconnectedController);
    BP32.forgetBluetoothKeys();
    BP32.enableVirtualDevice(false);
}

void loop() {
    bool dataUpdated = BP32.update();
    if (dataUpdated)
        processControllers();

    delay(100);
}
