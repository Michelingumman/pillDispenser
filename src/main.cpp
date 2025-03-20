#include <Arduino.h>
#include "images.h"
#include "secrets.h"
#include "animations.h"

#include <ESP32Servo.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <esp_pm.h> 
#include <esp_sleep.h>
#include <driver/gpio.h> // Required for gpio_hold functions

#define TOUCHPIN 10
#define SERVO_PIN 3
#define LED_PIN 8

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define SDA 6
#define SCL 7

#define SERVO_MIN_ANGLE 30    // Minimum safe angle
#define SERVO_MAX_ANGLE 10   // Maximum safe angle
#define SERVO_MOVE_DELAY 20 // Delay between each degree of movement

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Servo myservo;

bool servoState = false;
bool lastTouchState = false;
int currentServoAngle = SERVO_MIN_ANGLE;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// Global timing variables
unsigned long previousMillis = 0;

// Global variables
volatile bool touchDetected = false;
volatile unsigned long lastInterruptTime = 0;
bool touchInProgress = false;  // Flag to track if touch sequence is running
volatile int interruptCounter = 0;  // Counter for interrupt diagnostics

// Non-blocking delay function
bool hasitbeen(unsigned long interval) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        return true;
    }
    return false;
}

void moveServoSmooth(int targetAngle) {
    // Ensure target is within limits
    targetAngle = constrain(targetAngle, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    
    // Move servo smoothly to target
    if (currentServoAngle < targetAngle) {
        for (int angle = currentServoAngle; angle <= targetAngle; angle++) {
            myservo.write(angle);
            delay(SERVO_MOVE_DELAY);
        }
    } else {
        for (int angle = currentServoAngle; angle >= targetAngle; angle--) {
            myservo.write(angle);
            delay(SERVO_MOVE_DELAY);
        }

    }
    currentServoAngle = targetAngle;
}

// Define touch pin interrupt with proper debouncing
void IRAM_ATTR touchInterrupt() {
    // Simple interrupt debouncing
    unsigned long interruptTime = millis();
    interruptCounter++;  // Count all interrupts for diagnostics
    
    if (interruptTime - lastInterruptTime > 20) {  // Increase to 500ms for better debouncing
        touchDetected = true;
        lastInterruptTime = interruptTime;
    }
}

void setup() {
    // Reduce CPU frequency to 80MHz (from default 240MHz)
    // setCpuFrequencyMhz(80);

    Serial.begin(9600);
    Wire.begin(SDA, SCL);
    delay(10);

    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    myservo.setPeriodHertz(50);    // standard 50 hz servo

    // Initialize servo
    myservo.attach(SERVO_PIN);
    moveServoSmooth(SERVO_MIN_ANGLE);  // Move to initial position smoothly
    
    // Configure LED pin for hold during sleep - DO NOT include servo pin
    gpio_config_t io_conf = {};
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << LED_PIN); // Only configure LED pin, not servo
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    // Set up touch pin with interrupt and pull-down resistor
    pinMode(TOUCHPIN, INPUT_PULLDOWN);  // Add pull-down to prevent floating
    attachInterrupt(digitalPinToInterrupt(TOUCHPIN), touchInterrupt, RISING);
    
    // Enable wake-up on GPIO pin
    gpio_wakeup_enable((gpio_num_t)TOUCHPIN, GPIO_INTR_HIGH_LEVEL);
    esp_sleep_enable_gpio_wakeup();

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);  // Ensure LED is off initially

    // Initialize display with debug messages
    Serial.println("Initializing display...");
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
    }
    Serial.println("Display initialized successfully!");
    
    display.clearDisplay();
    display.setRotation(0);
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("Starting up...");
    display.display();
    delay(10);  // Show startup message for 1 second

    // Initialize random seed once at startup
    randomSeed(analogRead(0));
    
    // Print power saving mode info
    Serial.println("Power saving mode active - CPU at 80MHz with light sleep");
}

void loop() {
    // Make sure LED is explicitly OFF at the beginning of each loop iteration
    digitalWrite(LED_PIN, LOW);
    
    // Handle touch event (from interrupt) - only if no sequence is already running
    if (touchDetected && !touchInProgress) {
        detachInterrupt(digitalPinToInterrupt(TOUCHPIN));  // Disable interrupt during sequence
        touchInProgress = true;  // Prevent reentrance
        
        // Get current touch state and verify it's really HIGH
        bool currentTouchState = digitalRead(TOUCHPIN);
        Serial.print("Touch detected: ");
        Serial.println(currentTouchState);
        Serial.print("Interrupt count: ");
        Serial.println(interruptCounter);
        
        if (currentTouchState == HIGH) {
            // No need to be power efficient during touch sequence
            gpio_hold_dis((gpio_num_t)SERVO_PIN);
            
            display.clearDisplay();
            display.setCursor(0,0);
            display.setTextSize(1.5);
            display.println("Giving u some meds");
            display.display();

            // 1. Perform a sequence of servo movements
            Serial.println("Starting servo sequence");
            
            // First movement: Home position
            moveServoSmooth(SERVO_MAX_ANGLE);
            delay(1000);  // Pause at position
            
            moveServoSmooth(SERVO_MIN_ANGLE);
            delay(500);  // Pause at position

            moveServoSmooth(SERVO_MAX_ANGLE);
            delay(10);  // small pause for stability

            gpio_hold_dis((gpio_num_t)LED_PIN);
            // 2. Show scrolling text message
            int randomIndex = random(0, messageCount);
            String message = messages[randomIndex];
            Serial.print("Showing message: ");
            Serial.println(message);
            
            showScrollingText(display, message, 30); // Faster scrolling (30ms)
            
            // Wait for text scrolling to complete
            while (isAnimating()) {
                updateAnimation(display);
                delay(5);
            }
            
            // 3. Show dancing couple animation
            Serial.println("Starting dancing couple animation");
            startAnimation(drawDancingCouple, "frame1", "frame2", 20, 150, display);
            
            // Wait for dancing animation to complete
            while (isAnimating()) {
                updateAnimation(display);
                delay(5);
            }

            Serial.println("Touch sequence complete");
        }
        
        // Reset touch flags after sequence is complete
        touchDetected = false;
        touchInProgress = false;
        
        // Wait to make sure the touch is released before re-enabling interrupt
        while (digitalRead(TOUCHPIN) == HIGH) {
            delay(100);
        }
        delay(500);  // Additional debounce delay
        
        attachInterrupt(digitalPinToInterrupt(TOUCHPIN), touchInterrupt, RISING);
    }
    
    // If no animation is running, return to the power-efficient default state
    if (!isAnimating()) {
        // Ensure LED is off in default mode
        digitalWrite(LED_PIN, LOW);
        startAnimation(drawLadyAndGentleman, "small", "big", 100, 500, display);
    }
    
    // Update animation with power-saving (only in default state)
    if (isAnimating()) {
        // Get time until next animation frame
        unsigned long currentTime = millis();
        unsigned long timeSinceLastFrame = currentTime - animState.lastFrameTime;
        unsigned long timeToNextFrame = 0;
        
        if (timeSinceLastFrame < animState.delayMs) {
            timeToNextFrame = animState.delayMs - timeSinceLastFrame;
        }
        
        // First update the animation
        updateAnimation(display);
        
        // Then sleep until next frame if there's enough time
        if (timeToNextFrame > 20) { // Only sleep if it's worth it (> 20ms)
            // Hold only the LED pin state - do NOT hold the servo pin
            gpio_hold_en((gpio_num_t)LED_PIN);
            gpio_hold_en((gpio_num_t)SERVO_PIN);
            
            // Enable wake up from timer and touch pin
            esp_sleep_enable_timer_wakeup(timeToNextFrame * 1000); // microseconds
            esp_light_sleep_start();
            
            // After waking up, disable pin hold
            gpio_hold_dis((gpio_num_t)LED_PIN);
        }
    } else {
        // If nothing to animate, sleep for a short time
        delay(10);
    }
    
    // Debug output (every 5 seconds)
    static unsigned long lastDebugTime = 0;
    if (millis() - lastDebugTime > 5000) { // Print debug info every 5 seconds
        Serial.print("Touch pin state: ");
        Serial.println(digitalRead(TOUCHPIN));
        Serial.print("Animation running: ");
        Serial.println(isAnimating());
        Serial.print("Interrupt count: ");
        Serial.println(interruptCounter);
        Serial.println("-------------------");
        lastDebugTime = millis();
    }
}

