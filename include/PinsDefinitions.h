// I2S Configuration for NS4168
#define I2S_DOUT 22     // DIN pin on NS4168
#define I2S_BCLK 19     // BCLK pin on NS4168
#define I2S_LRC  33     // LRC pin on NS4168

#if defined(CONFIG_IDF_TARGET_ESP32S3)
#define PUZZLE_DISPLAY_PIXEL_PIN 2
#else
#define I2C_SDA                     26
#define I2C_SCL                     32

#define PUZZLE_DISPLAY_PIXEL_PIN    23

#define X_SERVO_PIN                 21
#define Y_SERVO_PIN                 25
#endif

