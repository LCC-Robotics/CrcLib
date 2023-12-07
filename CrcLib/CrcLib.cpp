#include "CrcLib.h"
#if defined(__AVR_ATmega1280__) \
    || defined(__AVR_ATmega2560__) /* Only if on the mega */

unsigned long CrcLib::_lastUpdateTime = 0;
unsigned int CrcLib::_deltaTime = 0;
unsigned int CrcLib::_hbCountdown = SCAN_COUNT_HB;
bool CrcLib::_commsLastConnected = false;
bool CrcLib::_buzzer = true;
const unsigned char CrcLib::CRC_LED_NEO = 32;
const unsigned char CrcLib::CRC_LED_ST = 34;
const unsigned char CrcLib::CRC_LED_FAIL = 39;
const unsigned char CrcLib::CRC_BUZZER = 46;
const unsigned char CrcLib::CRC_TXD_XBEE = 16;
const unsigned char CrcLib::CRC_RXD_XBEE = 17;
CrcUtility::CrcNeo CrcLib::_crcNeo = CrcUtility::CrcNeo(CRC_LED_NEO);
CrcUtility::CrcXbee CrcLib::_crcXbee = CrcUtility::CrcXbee();
CrcUtility::CrcBuzz CrcLib::_crcBuzz = CrcUtility::CrcBuzz();

struct CrcLib::ServoInfo CrcLib::_servos[12]
    = { { nullptr, 1000, 2000, false }, { nullptr, 1000, 2000, false },
          { nullptr, 1000, 2000, false }, { nullptr, 1000, 2000, false },
          { nullptr, 1000, 2000, false }, { nullptr, 1000, 2000, false },
          { nullptr, 1000, 2000, false }, { nullptr, 1000, 2000, false },
          { nullptr, 1000, 2000, false }, { nullptr, 1000, 2000, false },
          { nullptr, 1000, 2000, false }, { nullptr, 1000, 2000, false } };

unsigned char CrcLib::_pwmPins[12]
    = { CRC_PWM_1, CRC_PWM_2, CRC_PWM_3, CRC_PWM_4, CRC_PWM_5, CRC_PWM_6,
          CRC_PWM_7, CRC_PWM_8, CRC_PWM_9, CRC_PWM_10, CRC_PWM_11, CRC_PWM_12 };

CrcLib::CrcLib()
{
    _lastUpdateTime = 0;
    _commsLastConnected = false;
}

void CrcLib::Update()
{
    // Update time first
    UpdateTime();

    // Do all other updates after

    // Refresh data from remote
    float bat = GetBatteryVoltage();
    uint8_t status;
    if (bat >= 12.5) {
        status = 0b1010; // Green stable
    } else if (bat >= 12.) {
        status = 0b1111; // Orange stable
    } else if (bat >= 11.) {
        status = 0b0101; // Red stable
    } else {
        status = 0b0100; // Red - black - red - black - etc.
    }
    _crcXbee.UpdateData(status);

    if (!_crcXbee.IsCommValid() && _commsLastConnected) {
        // The communication was previously active but now is not connected
        _commsLastConnected = false;
        ShowColorPattern(CrcUtility::PATTERN_DISCONNECTED, false);
        if (_buzzer) {
            PlayTune(CrcUtility::TUNE_DISCONNECTED, false);
        }
    } else if (_crcXbee.IsCommValid() && !_commsLastConnected) {
        // The communication was not active but now it is
        _commsLastConnected = true;
        ShowColorPattern(CrcUtility::PATTERN_CONNECTED, false);
        if (_buzzer) {
            PlayTune(CrcUtility::TUNE_CONNECTED, false);
        }
    }

    // Update the neopixel
    _crcNeo.Update(_deltaTime);

    // Update the buzzer
    _crcBuzz.Update(_deltaTime);

    // Status Heartbeat
    CrcLib::StatusHeartbeat();
}

void CrcLib::PrintControllerState() { return _crcXbee.State().PrintPayload(); }

unsigned int CrcLib::GetDeltaTimeMillis() { return _deltaTime / 1000; }

unsigned int CrcLib::GetDeltaTimeMicros() { return _deltaTime; }

void CrcLib::StatusHeartbeat()
{
    if (_hbCountdown) {
        _hbCountdown--;
    } else {
        _hbCountdown = SCAN_COUNT_HB;
        digitalWrite(CRC_LED_ST, !digitalRead(CRC_LED_ST));
    }
}

void CrcLib::UpdateTime()
{
    if (_lastUpdateTime != 0) {
        unsigned long currentTime = micros();

        // In the case micros overflows, the delta time will be 0 for only one
        // update
        _deltaTime = (currentTime < _lastUpdateTime)
            ? 0
            : currentTime - _lastUpdateTime;

        _lastUpdateTime = currentTime;
    } else {
        // This is the first update
        _lastUpdateTime = micros();
        _deltaTime = 0;
    }
}

bool CrcLib::ReadDigitalChannel(BUTTON channel)
{
    return _crcXbee.ReadDigitalChannel(channel);
}

int8_t CrcLib::ReadAnalogChannel(ANALOG channel)
{
    return _crcXbee.ReadAnalogChannel(channel) - 128;
}

void CrcLib::MoveTank(int8_t leftChannel,
    int8_t rightChannel,
    unsigned char leftMotor,
    unsigned char rightMotor)
{
    SetPwmOutput(leftMotor, leftChannel);
    SetPwmOutput(rightMotor, rightChannel);
}

void CrcLib::MoveTank(ANALOG leftChannel,
    ANALOG rightChannel,
    unsigned char leftMotor,
    unsigned char rightMotor)
{
    MoveTank(ReadAnalogChannel(leftChannel), ReadAnalogChannel(rightChannel),
        leftMotor, rightMotor);
}

void CrcLib::MoveTank(int8_t leftChannel,
    int8_t rightChannel,
    unsigned char frontLeftMotor,
    unsigned char backLeftMotor,
    unsigned char frontRightMotor,
    unsigned char backRightMotor)
{
    SetPwmOutput(frontLeftMotor, leftChannel);
    SetPwmOutput(backLeftMotor, leftChannel);
    SetPwmOutput(frontRightMotor, rightChannel);
    SetPwmOutput(backRightMotor, rightChannel);
}

void CrcLib::MoveTank(ANALOG leftChannel,
    ANALOG rightChannel,
    unsigned char frontLeftMotor,
    unsigned char backLeftMotor,
    unsigned char frontRightMotor,
    unsigned char backRightMotor)
{
    MoveTank(ReadAnalogChannel(leftChannel), ReadAnalogChannel(rightChannel),
        frontLeftMotor, backLeftMotor, frontRightMotor, backRightMotor);
}

void CrcLib::MoveArcade(int8_t forwardChannel,
    int8_t yawChannel,
    unsigned char leftMotor,
    unsigned char rightMotor)
{
    int8_t left = constrain(forwardChannel - yawChannel, -128,
        127); // Determines the power of the left wheels
    int8_t right = constrain(forwardChannel + yawChannel, -128,
        127); // Determines the power of the right wheels

    SetPwmOutput(leftMotor, left);
    SetPwmOutput(rightMotor, right);
}

void CrcLib::MoveArcade(ANALOG forwardChannel,
    ANALOG yawChannel,
    unsigned char leftMotor,
    unsigned char rightMotor)
{
    MoveArcade(ReadAnalogChannel(forwardChannel), ReadAnalogChannel(yawChannel),
        leftMotor, rightMotor);
}

void CrcLib::MoveArcade(int8_t forwardChannel,
    int8_t yawChannel,
    unsigned char frontLeftMotor,
    unsigned char backLeftMotor,
    unsigned char frontRightMotor,
    unsigned char backRightMotor)
{
    int8_t left = constrain(forwardChannel - yawChannel, -128,
        127); // Determines the power of the left wheels
    int8_t right = constrain(forwardChannel + yawChannel, -128,
        127); // Determines the power of the right wheels

    SetPwmOutput(frontLeftMotor, left);
    SetPwmOutput(backLeftMotor, left);
    SetPwmOutput(frontRightMotor, right);
    SetPwmOutput(backRightMotor, right);
}

void CrcLib::MoveArcade(ANALOG forwardChannel,
    ANALOG yawChannel,
    unsigned char frontLeftMotor,
    unsigned char backLeftMotor,
    unsigned char frontRightMotor,
    unsigned char backRightMotor)
{
    MoveArcade(ReadAnalogChannel(forwardChannel), ReadAnalogChannel(yawChannel),
        frontLeftMotor, backLeftMotor, frontRightMotor, backRightMotor);
}

void CrcLib::MoveHolonomic(int8_t forwardChannel,
    int8_t yawChannel,
    int8_t strafeChannel,
    unsigned char frontLeftMotor,
    unsigned char backLeftMotor,
    unsigned char frontRightMotor,
    unsigned char backRightMotor)
{
    int8_t frontLeft = constrain(forwardChannel - yawChannel - strafeChannel,
        -128, 127); // Determines the power of the front left wheel
    int8_t backLeft = constrain(forwardChannel - yawChannel + strafeChannel,
        -128, 127); // Determines the power of the front left wheel
    int8_t frontRight = constrain(forwardChannel + yawChannel + strafeChannel,
        -128, 127); // Determines the power of the front left wheel
    int8_t backRight = constrain(forwardChannel + yawChannel - strafeChannel,
        -128, 127); // Determines the power of the right wheels

    SetPwmOutput(frontLeftMotor, frontLeft);
    SetPwmOutput(backLeftMotor, backLeft);
    SetPwmOutput(frontRightMotor, frontRight);
    SetPwmOutput(backRightMotor, backRight);
}

void CrcLib::MoveHolonomic(ANALOG forwardChannel,
    ANALOG yawChannel,
    ANALOG strafeChannel,
    unsigned char frontLeftMotor,
    unsigned char backLeftMotor,
    unsigned char frontRightMotor,
    unsigned char backRightMotor)
{
    MoveHolonomic(ReadAnalogChannel(forwardChannel),
        ReadAnalogChannel(yawChannel), ReadAnalogChannel(strafeChannel),
        frontLeftMotor, backLeftMotor, frontRightMotor, backRightMotor);
}

void CrcLib::Initialize() { Initialize(true); }

void CrcLib::Initialize(bool buzzer)
{
    // Setup neopixel
    _crcNeo.Initialize();

    // Setup buzzer
    _crcBuzz.Initialize(CRC_BUZZER);

    // Enable or disable buzzer
    _buzzer = buzzer;

    SetupPins();

    // Setup xBee communication
    _crcXbee.Initialize(Serial2);

    // Start metro tune/pattern
    ShowColorPattern(CrcUtility::PATTERN_STARTUP, false);
    if (buzzer)
        PlayTune(CrcUtility::TUNE_METRO, false);
}

void CrcLib::SetupPins()
{
    // Set all the exposed pwm pins to be outputs here
    pinMode(CRC_PWM_1, OUTPUT);
    pinMode(CRC_PWM_2, OUTPUT);
    pinMode(CRC_PWM_3, OUTPUT);
    pinMode(CRC_PWM_4, OUTPUT);
    pinMode(CRC_PWM_5, OUTPUT);
    pinMode(CRC_PWM_6, OUTPUT);
    pinMode(CRC_PWM_7, OUTPUT);
    pinMode(CRC_PWM_8, OUTPUT);
    pinMode(CRC_PWM_9, OUTPUT);
    pinMode(CRC_PWM_10, OUTPUT);
    pinMode(CRC_PWM_11, OUTPUT);
    pinMode(CRC_PWM_12, OUTPUT);

    // Set all the analog pins to be treated as inputs
    pinMode(CRC_ANA_1, INPUT);
    pinMode(CRC_ANA_2, INPUT);
    pinMode(CRC_ANA_3, INPUT);
    pinMode(CRC_ANA_4, INPUT);

    pinMode(CRC_LED_ST, OUTPUT);
    pinMode(CRC_LED_FAIL, OUTPUT);
    pinMode(CRC_BUZZER, OUTPUT);
    pinMode(CRC_VBATT, INPUT);

    pinMode(CRC_ENCO_A, INPUT);
    pinMode(CRC_ENCO_B, INPUT);
}

void CrcLib::SetDigitalPinMode(unsigned char pin, unsigned char mode)
{
    if (!IsSafeDigitalPin(pin))
        StopEverythingFromError(ERROR_UNSAFE_DIGITAL_PIN);

    pinMode(pin, mode);
}

unsigned char CrcLib::GetDigitalInput(unsigned char pin)
{
    if (!IsSafeDigitalPin(pin))
        StopEverythingFromError(ERROR_UNSAFE_DIGITAL_PIN);

    return digitalRead(pin);
}

void CrcLib::SetDigitalOutput(unsigned char pin, unsigned char value)
{
    if (!IsSafeDigitalPin(pin))
        StopEverythingFromError(ERROR_UNSAFE_DIGITAL_PIN);

    if (value != LOW && value != HIGH)
        StopEverythingFromError(ERROR_INCORRECT_DIGITAL_VALUE);

    digitalWrite(pin, value);
}

bool CrcLib::IsSafeDigitalPin(unsigned char pin)
{
    switch (pin) {
    case CRC_DIG_1:
    case CRC_DIG_2:
    case CRC_DIG_3:
    case CRC_DIG_4:
    case CRC_DIG_5:
    case CRC_DIG_6:
    case CRC_DIG_7:
    case CRC_DIG_8:
    case CRC_DIG_9:
    case CRC_DIG_10:
    case CRC_DIG_11:
    case CRC_DIG_12:
        return true;
    default:
        return false;
    }
}

void CrcLib::SetPwmOutput(unsigned char pin, char value)
{
    if (!IsSafePwmPin(pin))
        StopEverythingFromError(ERROR_UNSAFE_PWM_PIN);

    int index = PinToServoIndex(pin);

    ServoInfo* servo = &(_servos[index]);

    if (servo->servo == nullptr)
        StopEverythingFromError(ERROR_SERVO_NOT_INITIALIZED);

    int mapped_value = (servo->minPulseWidth + servo->maxPulseWidth) / 2;

    if (value
        != 0) // Évite un potentiel lag si le joystick est au neutre, p-t que ça
              //  va se transformer en genre 1501 et envoyer un faible signal...
        mapped_value = map(value * servo->reverse, -128, 127,
            servo->minPulseWidth, servo->maxPulseWidth);

    servo->servo->writeMicroseconds(mapped_value);
}

void CrcLib::InitializePwmOutput(unsigned char pin)
{
    InitializePwmOutput(pin, 1000, 2000, false);
}

void CrcLib::InitializePwmOutput(
    unsigned char pin, int minPulseWidth, int maxPulseWidth)
{
    InitializePwmOutput(pin, minPulseWidth, maxPulseWidth, false);
}

void CrcLib::InitializePwmOutput(unsigned char pin, bool reverse)
{
    InitializePwmOutput(pin, 1000, 2000, reverse);
}

void CrcLib::InitializePwmOutput(
    unsigned char pin, int minPulseWidth, int maxPulseWidth, bool reverse)
{
    if (!IsSafePwmPin(pin))
        StopEverythingFromError(ERROR_UNSAFE_PWM_PIN);

    if (minPulseWidth >= maxPulseWidth || minPulseWidth <= 0
        || maxPulseWidth <= 0)
        StopEverythingFromError(ERROR_INCORRECT_SERVO_PULSEWIDTH);

    int index = PinToServoIndex(pin);

    ServoInfo* servo = &(_servos[index]);

    if (servo->servo != nullptr)
        StopEverythingFromError(ERROR_SERVO_ALREADY_INITIALIZED);

    servo->servo = new Servo();
    servo->servo->attach(pin, minPulseWidth, maxPulseWidth);
    servo->minPulseWidth = minPulseWidth;
    servo->maxPulseWidth = maxPulseWidth;
    servo->reverse = reverse ? -1 : 1;
}

int CrcLib::PinToServoIndex(unsigned char pin)
{
    switch (pin) {
    case CRC_PWM_1: return 0;
    case CRC_PWM_2: return 1;
    case CRC_PWM_3: return 2;
    case CRC_PWM_4: return 3;
    case CRC_PWM_5: return 4;
    case CRC_PWM_6: return 5;
    case CRC_PWM_7: return 6;
    case CRC_PWM_8: return 7;
    case CRC_PWM_9: return 8;
    case CRC_PWM_10: return 9;
    case CRC_PWM_11: return 10;
    case CRC_PWM_12: return 11;
    default: StopEverythingFromError(ERROR_UNSAFE_PWM_PIN);
    }
}

bool CrcLib::IsSafePwmPin(unsigned char pin)
{
    switch (pin) {
    case CRC_PWM_1:
    case CRC_PWM_2:
    case CRC_PWM_3:
    case CRC_PWM_4:
    case CRC_PWM_5:
    case CRC_PWM_6:
    case CRC_PWM_7:
    case CRC_PWM_8:
    case CRC_PWM_9:
    case CRC_PWM_10:
    case CRC_PWM_11:
    case CRC_PWM_12:
        return true;
    default:
        return false;
    }
}

unsigned int CrcLib::GetAnalogInput(unsigned char pin)
{
    if (!IsSafeAnalogPin(pin))
        StopEverythingFromError(ERROR_UNSAFE_ANALOG_PIN);

    return analogRead(pin);
}

bool CrcLib::IsSafeAnalogPin(unsigned char pin)
{
    switch (pin) {
    case CRC_ANA_1:
    case CRC_ANA_2:
    case CRC_ANA_3:
    case CRC_ANA_4:
        return true;
    default:
        return false;
    }
}

/** Read battery voltage. */
float CrcLib::GetBatteryVoltage() { return GetBatteryVoltage(1.0); }

/** Read battery voltage, with correction factor to account of resistors
 * imprecision. */
float CrcLib::GetBatteryVoltage(float correction)
{
    return analogRead(CRC_VBATT) * (5 * 4 / 1023.0) * correction;
}

inline void CrcLib::StopEverythingFromError(const ColorDuration colors[])
{
    CrcLib::StopEverythingFromError(nullptr, colors);
}

void CrcLib::StopEverythingFromError(const Note notes[], const ColorDuration colors[])
{

    // Stop all current output, since we don't want to freeze things while
    // motors are still running and whatnot
    StopAllOutput();

    // Depending on the error code, display (LED / buzzer) the appropriate
    // pattern
    digitalWrite(CRC_LED_FAIL, HIGH);
    ShowColorPattern(colors, true);

    if (notes != nullptr)
        PlayTune(notes, true);

    // Wait indefinitely (maybe check certain important things or update LED
    // flashes, but never leave this function once we've entered)
    while (true) {
        // Still update the time, but no pins
        UpdateTime();
        _crcNeo.Update(_deltaTime);
        _crcBuzz.Update(_deltaTime);
    }
}

void CrcLib::StopEverythingFromError(unsigned char errorCode)
{
    // Debug
    Serial.println("Stopped from error:");
    Serial.println(errorCode);

    if (_buzzer) {
        StopEverythingFromError(GetErrorTune(errorCode), GetErrorLightPattern(errorCode));
    } else {
        StopEverythingFromError(GetErrorLightPattern(errorCode));
    }
}

const CrcUtility::Note* CrcLib::GetErrorTune(unsigned char errorCode)
{
    switch (errorCode) {
    case ERROR_UNSAFE_DIGITAL_PIN:
    case ERROR_UNSAFE_ANALOG_PIN:
    case ERROR_UNSAFE_PWM_PIN:
    case ERROR_UNSAFE_PWM_DIG_PIN:
        return CrcUtility::TUNE_PIN_ERROR;
    case false:
        return CrcUtility::TUNE_SPARE;
    case ERROR_INCORRECT_DIGITAL_VALUE:
        return CrcUtility::TUNE_VALUE_ERROR;
    case ERROR_SERVO_ALREADY_INITIALIZED:
    case ERROR_SERVO_NOT_INITIALIZED:
    case ERROR_INCORRECT_SERVO_PULSEWIDTH:
        return CrcUtility::TUNE_SERVO_ERROR;
    default:
        return CrcUtility::TUNE_TEST;
    }
}

const CrcUtility::ColorDuration* CrcLib::GetErrorLightPattern(
    unsigned char errorCode)
{
    switch (errorCode) {
    case ERROR_SPARE10:
    case ERROR_UNSAFE_DIGITAL_PIN:
    case ERROR_SERVO_ALREADY_INITIALIZED:
        return CrcUtility::PATTERN_ERROR1;
    case ERROR_SPARE11:
    case ERROR_UNSAFE_ANALOG_PIN:
    case ERROR_SERVO_NOT_INITIALIZED:
        return CrcUtility::PATTERN_ERROR2;
    case ERROR_UNSAFE_PWM_PIN:
    case ERROR_SPARE12:
    case ERROR_INCORRECT_SERVO_PULSEWIDTH:
        return CrcUtility::PATTERN_ERROR3;
    case ERROR_UNSAFE_PWM_DIG_PIN:
    case ERROR_INCORRECT_DIGITAL_VALUE:
        return CrcUtility::PATTERN_ERROR4;
    default:
        return CrcUtility::PATTERN_TEST;
    }
}

void CrcLib::StopAllOutput()
{
    digitalWrite(CRC_DIG_1, LOW);
    digitalWrite(CRC_DIG_2, LOW);
    digitalWrite(CRC_DIG_3, LOW);
    digitalWrite(CRC_DIG_4, LOW);
    digitalWrite(CRC_DIG_5, LOW);
    digitalWrite(CRC_DIG_6, LOW);
    digitalWrite(CRC_DIG_7, LOW);
    digitalWrite(CRC_DIG_8, LOW);
    digitalWrite(CRC_DIG_9, LOW);
    digitalWrite(CRC_DIG_10, LOW);
    digitalWrite(CRC_DIG_11, LOW);
    digitalWrite(CRC_DIG_12, LOW);

    // If no servo, then set output to 0,
    // but if servo, then set servo to not move.
    for (int i = 0; i < 12; i++) {
        ServoInfo* servo = &(_servos[i]);

        if (servo->servo == nullptr) {
            analogWrite(_pwmPins[i], 0);
            continue;
        } else {
            SetPwmOutput(_pwmPins[i], 0);
        }
    }
}

bool CrcLib::IsCommValid() { return _crcXbee.IsCommValid(); }

void CrcLib::Timer::Start(uint32_t delay)
{
    _started = millis();
    _delay = delay;
}
bool CrcLib::Timer::IsFinished() const
{
    return ((uint32_t)millis()) - _started > _delay && _delay != 0;
}
bool CrcLib::Timer::IsWaiting() const { return _delay != 0 && !IsFinished(); }
void CrcLib::Timer::Next() { _started += _delay; }

void CrcLib::PlayTune(const Note notes[], bool repeat)
{
    static SimpleTune static_tune;

    static_tune = SimpleTune(notes, repeat);
    PlayTune(&static_tune);
}

void CrcLib::PlayTune(Tune* tune) { _crcBuzz.StartTune(tune); }

void CrcLib::SetColor(const Color color) { _crcNeo.SetColor(color); }

void CrcLib::ShowColorPattern(const ColorDuration pattern[], bool repeat)
{
    static SimpleColorPattern static_pattern;

    static_pattern = SimpleColorPattern(pattern, repeat);
    ShowColorPattern(&static_pattern);
}

void CrcLib::ShowColorPattern(ColorPattern* pattern)
{
    _crcNeo.StartPattern(pattern);
}

#endif /* Black box */
