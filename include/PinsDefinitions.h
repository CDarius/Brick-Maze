#if defined(M5STACK_ATOM_S3)
#define PUZZLE_DISPLAY_PIXEL_PIN 2
#elif defined(M5STACK_ATOM_ECHO)
#define I2C_SDA                     26
#define I2C_SCL                     32

#define PUZZLE_DISPLAY_PIXEL_PIN    23

#define X_SERVO_PIN                 21
#define Y_SERVO_PIN                 25

// I2S Configuration for NS4168
#define I2S_DOUT 22     // DIN pin on NS4168
#define I2S_BCLK 19     // BCLK pin on NS4168
#define I2S_LRC  33     // LRC pin on NS4168

#elif defined(SEEED_XIAO_ESP32S3)

#define I2C_SDA                     5
#define I2C_SCL                     6

#define PUZZLE_DISPLAY_PIXEL_PIN    4

#define X_SERVO_PIN                 7
#define Y_SERVO_PIN                 8

#define I2S_DOUT                    3
#define I2S_BCLK                    2
#define I2S_LRC                     1

#define REMOTE_CONTROLLER_UART_RX   44
#define REMOTE_CONTROLLER_UART_TX   43
#endif

