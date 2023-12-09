#ifndef CrcLib_h
#define CrcLib_h

#include "CrcXbee.h"
#include <Arduino.h>
#include <Servo.h>
#if defined(__AVR_ATmega1280__) \
    || defined(__AVR_ATmega2560__) /* Only on the custom board */
#include "CrcBuzz.h"
#include "CrcNeo.h"

//---------- Reexport -------------------------
using CrcUtility::ANALOG;
using CrcUtility::BUTTON;

using CrcUtility::Color;
using CrcUtility::ColorDuration;
using CrcUtility::ColorPattern;
using CrcUtility::Note;
using CrcUtility::SimpleColorPattern;
using CrcUtility::SimpleTune;
using CrcUtility::Tune;

using CrcUtility::NO_COLOR;

using CrcUtility::BLUE_HIGH;
using CrcUtility::BLUE_LOW;
using CrcUtility::CYAN_HIGH;
using CrcUtility::CYAN_LOW;
using CrcUtility::GREEN_HIGH;
using CrcUtility::GREEN_LOW;
using CrcUtility::MAGENTA_HIGH;
using CrcUtility::MAGENTA_LOW;
using CrcUtility::ORANGE_HIGH;
using CrcUtility::RED_HIGH;
using CrcUtility::RED_LOW;
using CrcUtility::WHITE_HIGH;
using CrcUtility::WHITE_LOW;
using CrcUtility::YELLOW_HIGH;
using CrcUtility::YELLOW_LOW;

//---------- Constants or Defaults ------------
// The number of program scans between CRC_LED_ST state changes.
// A >20ms scan time flashes every second or more, indicating a potential
// code problem
static const unsigned int SCAN_COUNT_HB = 100;

// Exposed analog pins
#define CRC_ANA_1 A0
#define CRC_ANA_2 A1
#define CRC_ANA_3 A2
#define CRC_ANA_4 A3

// Exposed digital pins
#define CRC_DIG_1 22
#define CRC_DIG_2 23
#define CRC_DIG_3 24
#define CRC_DIG_4 25
#define CRC_DIG_5 26
#define CRC_DIG_6 27
#define CRC_DIG_7 28
#define CRC_DIG_8 29
#define CRC_DIG_9 30
#define CRC_DIG_10 31
#define CRC_DIG_11 A4
#define CRC_DIG_12 A5

// Exposed pwm pins
#define CRC_PWM_1 4
#define CRC_PWM_2 5
#define CRC_PWM_3 6
#define CRC_PWM_4 7
#define CRC_PWM_5 8
#define CRC_PWM_6 9
#define CRC_PWM_7 10
#define CRC_PWM_8 11
#define CRC_PWM_9 12
#define CRC_PWM_10 13
#define CRC_PWM_11 44
#define CRC_PWM_12 45

// Temperature
#define CRC_LM35D A14

// Battery voltage
#define CRC_VBATT A15

// SPI pins
#define CRC_SPI_MISO 50
#define CRC_SPI_MOSI 51
#define CRC_SPI_SCK 52
#define CRC_SPI_SS 53

// I2C pins, also interrupt
#define CRC_I2C_SDA 20 // SDA
#define CRC_I2C_SCL 21 // SCL

// Serial pins, also interrupt
#define CRC_SERIAL_TXD1 18 // TXD1
#define CRC_SERIAL_RXD1 19 // RXD1

// Encoder pins, interrupt
#define CRC_ENCO_A 2
#define CRC_ENCO_B 3

// ERROR condes
#define ERROR_UNSAFE_DIGITAL_PIN 1
#define ERROR_UNSAFE_ANALOG_PIN 2
#define ERROR_UNSAFE_PWM_PIN 3
#define ERROR_UNSAFE_PWM_DIG_PIN 4

#define ERROR_SPARE10 10
#define ERROR_SPARE11 11
#define ERROR_SPARE12 12

#define ERROR_INCORRECT_DIGITAL_VALUE 20

#define ERROR_SERVO_ALREADY_INITIALIZED 30
#define ERROR_SERVO_NOT_INITIALIZED 31
#define ERROR_INCORRECT_SERVO_PULSEWIDTH 32

//---------------------------------------------

class CrcLib {
public:
    class Timer {
    public:
        void Start(uint32_t delay);
        bool IsFinished() const;
        bool IsWaiting() const;
        void Next();

    private:
        uint32_t _started = 0;
        uint32_t _delay = 0;
    };

    /** Default constructor. */
    CrcLib();

    static void Initialize();
    static void Initialize(bool buzzer);

    /** Takes care of any calls or calculations that have to be done
     * continuously (every loop). If any features requiring a continuous update
     * are being used, such as automatically binding channels to ports, this
     * function should be called once in the "loop" function of your .ino file.
     */
    static void Update();

    // Returns the time in millis since the last update.
    // This has a strong possibility if being very small (0 or 1)
    // if nothing else is happening in the loop, in which case
    // the micros function should be used instead.
    static unsigned int GetDeltaTimeMillis();

    // Get the state of the controller (CRConnect)
    static void PrintControllerState();

    // Returns the time in micros since the last update.
    static unsigned int GetDeltaTimeMicros();

    /** Sets the pin mode for a digital pin.
     * Should be "INPUT", "OUTPUT", or "INPUT_PULLUP".
     */
    static void SetDigitalPinMode(unsigned char pin, unsigned char mode);

    /** Gets the input from a digital pin. */
    static unsigned char GetDigitalInput(unsigned char pin);

    /** Sets the value of a digital pin.
     * Should be "LOW" or "HIGH".
     */
    static void SetDigitalOutput(unsigned char pin, unsigned char value);

    /** Sets the value of a digital pin. */
    static void SetPwmOutput(unsigned char pin, char value);

    static void InitializePwmOutput(unsigned char pin);

    static void InitializePwmOutput(
        unsigned char pin, int minPulseWidth, int maxPulseWidth);

    static void InitializePwmOutput(unsigned char pin, bool reverse);

    static void InitializePwmOutput(
        unsigned char pin, int minPulseWidth, int maxPulseWidth, bool reverse);

    /** Gets the input from an analog pin. */
    static unsigned int GetAnalogInput(unsigned char pin);

    /** Read battery voltage. */
    static float GetBatteryVoltage();

    /* Correction allows to compensate tiny resistance difference between
     * everyboards. A perfect board would have a correction of 1.0 */
    static float GetBatteryVoltage(float correction);

    // Reads a digital channel value from the remote
    // To be used with btn::channel enum
    static bool ReadDigitalChannel(BUTTON channel);

    // Reads an analog (-127 to 128) value from the joysticks
    // To be used with analog::channel enum
    static int8_t ReadAnalogChannel(ANALOG channel);

    // Sets two (-128 to 127) values to two motors
    // Make sure to use IntializePWMOutput first for each motor!
    static void MoveTank(int8_t leftChannel,
        int8_t rightChannel,
        unsigned char leftMotor,
        unsigned char rightMotor);

    // Sets two analog channels to two motors
    // Make sure to use IntializePWMOutput first for each motor!
    static void MoveTank(ANALOG leftChannel,
        ANALOG rightChannel,
        unsigned char leftMotor,
        unsigned char rightMotor);

    // Sets two (-128 to 127) values to four motors
    // Make sure to use IntializePWMOutput first for each motor!
    // Motors on each side are sent the same command
    static void MoveTank(int8_t leftChannel,
        int8_t rightChannel,
        unsigned char frontLeftMotor,
        unsigned char backLeftMotor,
        unsigned char frontRightMotor,
        unsigned char backRightMotor);

    // Sets two analog channels to four motors
    // Make sure to use IntializePWMOutput first for each motor!
    // Motors on each side are sent the same command
    static void MoveTank(ANALOG leftChannel,
        ANALOG rightChannel,
        unsigned char frontLeftMotor,
        unsigned char backLeftMotor,
        unsigned char frontRightMotor,
        unsigned char backRightMotor);

    // Sets two (-128 to 127) values to two motors in an arcade fashion
    // fChannel corresponds to the forwards/backwards input
    // yChannel corresponds to the yaw (rotation) channel
    // Make sure to use IntializePWMOutput first for each motor!
    static void MoveArcade(int8_t forwardChannel,
        int8_t yawChannel,
        unsigned char leftMotor,
        unsigned char rightMotor);

    // Sets two analog channels to two motors in an arcade fashion
    // fChannel corresponds to the forwards/backwards input
    // yChannel corresponds to the yaw (rotation) channel
    // Make sure to use IntializePWMOutput first for each motor!
    static void MoveArcade(ANALOG forwardChannel,
        ANALOG yawChannel,
        unsigned char leftMotor,
        unsigned char rightMotor);

    // Sets two (-128 to 127) values to four motors in an arcade fashion
    // fChannel corresponds to the forwards/backwards input
    // yChannel corresponds to the yaw (rotation) channel
    // Make sure to use IntializePWMOutput first for each motor!
    static void MoveArcade(int8_t forwardChannel,
        int8_t yawChannel,
        unsigned char frontLeftMotor,
        unsigned char backLeftMotor,
        unsigned char frontRightMotor,
        unsigned char backRightMotor);

    // Sets two analog channels to four motors in an arcade fashion
    // fChannel corresponds to the forwards/backwards input
    // yChannel corresponds to the yaw (rotation) channel
    // Make sure to use IntializePWMOutput first for each motor!
    static void MoveArcade(ANALOG forwardChannel,
        ANALOG yawChannel,
        unsigned char frontLeftMotor,
        unsigned char backLeftMotor,
        unsigned char frontRightMotor,
        unsigned char backRightMotor);

    // Sets two analog channels to four motors in an holonomic fashion, to be
    // used with omni wheels forwardChannel corresponds to the
    // forwards/backwards input yawChannel corresponds to the yaw (rotation)
    // channel strafeChannel corresponds to the strafing (side to side) channel
    // Make sure to use IntializePWMOutput first for each motor!
    static void MoveHolonomic(int8_t forwardChannel,
        int8_t yawChannel,
        int8_t strafeChannel,
        unsigned char frontLeftMotor,
        unsigned char backLeftMotor,
        unsigned char frontRightMotor,
        unsigned char backRightMotor);

    // Sets two analog channels to four motors in an holonomic fashion, to be
    // used with omni wheels forwardChannel corresponds to the
    // forwards/backwards input yawChannel corresponds to the yaw (rotation)
    // channel strafeChannel corresponds to the strafing (side to side) channel
    // Make sure to use IntializePWMOutput first for each motor!
    static void MoveHolonomic(ANALOG forwardChannel,
        ANALOG yawChannel,
        ANALOG strafeChannel,
        unsigned char frontLeftMotor,
        unsigned char backLeftMotor,
        unsigned char frontRightMotor,
        unsigned char backRightMotor);

    // Returns TRUE when communication with CrcConnect is active
    static bool IsCommValid();

    /** Stops all stuff that's currently happening and displays some form of
     * error output. */
    static void StopEverythingFromError(const Note notes[], const ColorDuration colors[]);
    static void StopEverythingFromError(const ColorDuration colors[]);

    // Play a tune on the CrcDuino buzzer
    static void PlayTune(const Note notes[], bool repeat);
    static void PlayTune(Tune* tune);

    // Display a color on the NeoPixel
    static void SetColor(Color color);

    // Display a light pattern on the NeoPixel
    static void ShowColorPattern(const ColorDuration pattern[], bool repeat);
    static void ShowColorPattern(ColorPattern* pattern);

private:
    struct ServoInfo {
        Servo* servo;
        int minPulseWidth;
        int maxPulseWidth;
        int reverse;
    };

    /** Sets up any pin initialization stuff. */
    static void SetupPins();

    /** Updates the time and delta time variables. */
    static void UpdateTime();

    /** Stops all stuff that's currently happening and displays some form of
     * error output. */
    static void StopEverythingFromError(unsigned char errorCode);

    static const Note* GetErrorTune(unsigned char errorCode);

    static const ColorDuration* GetErrorLightPattern(unsigned char errorCode);

    // Reset all outputs that were potentially set to 0
    static void StopAllOutput();

    static constexpr int PinToServoIndex(unsigned char pin);

    /** Check if a given pin is an allowed digital pin. */
    static constexpr bool IsSafeDigitalPin(unsigned char pin);

    /** Check if a given pin is an allowed pwm pin. */
    static constexpr bool IsSafePwmPin(unsigned char pin);

    /** Checks if a given pin is an allowed analog pin. */
    static constexpr bool IsSafeAnalogPin(unsigned char pin);

    // Flashes the CRC_LED_ST to indicate program is not stuck somewhere
    static void StatusHeartbeat();

    static unsigned int _hbCountdown;

    // The time in microseconds at the beginning of the current update. Used to
    // determine the delta time.
    static unsigned long _lastUpdateTime;

    // The time in microseconds since the last update
    static unsigned int _deltaTime;

    // Last known communication state
    static bool _commsLastConnected;

    // Buzz on error?
    static bool _buzzer;

    static ServoInfo _servos[12];

    static const unsigned char _pwmPins[12];

    /** Neopixel pin */
    static constexpr unsigned char CRC_LED_NEO = 32;

    /** Status LED pin, green. */
    static constexpr unsigned char CRC_LED_ST = 34;

    /** Fail LED pin, red. */
    static constexpr unsigned char CRC_LED_FAIL = 39;

    /** Buzzer pin */
    static constexpr unsigned char CRC_BUZZER = 46;

    /** 2560 output going to TXD pin of the Board XBee */
    static constexpr unsigned char CRC_TXD_XBEE = 16;

    /** 2560 input coming from RXD pin of the Board XBee */
    static constexpr unsigned char CRC_RXD_XBEE = 17;

    static CrcUtility::CrcNeo _crcNeo;

    static CrcUtility::CrcXbee _crcXbee;

    static CrcUtility::CrcBuzz _crcBuzz;
};
#endif /* Black box */
#endif
