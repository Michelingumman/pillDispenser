#include <Arduino.h>
#include "images.h"
#include "secrets.h"
#include "animations.h"

#include <ESP32Servo.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define TOUCHPIN 10
#define SERVO_PIN 3
#define LED_PIN 8

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define SDA 6
#define SCL 7

#define SERVO_MIN_ANGLE 5    // Minimum safe angle
#define SERVO_MAX_ANGLE 175   // Maximum safe angle
#define SERVO_MOVE_DELAY 5  // Delay between each degree of movement

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Servo myservo;

bool servoState = false;
bool lastTouchState = false;
int currentServoAngle = SERVO_MIN_ANGLE;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

// Global timing variables
unsigned long previousMillis = 0;

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

void setup() {
    Serial.begin(9600);
    Wire.begin(SDA, SCL);
    delay(100);

    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    myservo.setPeriodHertz(50);    // standard 50 hz servo

    // Initialize servo
    myservo.attach(SERVO_PIN);
    moveServoSmooth(SERVO_MIN_ANGLE);  // Move to initial position smoothly

    // Set up touch pin
    pinMode(TOUCHPIN, INPUT);

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
    delay(100);  // Show startup message for 1 second

    // Initialize random seed once at startup
    randomSeed(analogRead(0));
}

void loop() {
    // Read the touch sensor state
    bool currentTouchState = digitalRead(TOUCHPIN);
    Serial.println(currentTouchState);
    
    // If touch is HIGH, toggle between positions and start animation sequence
    if (currentTouchState == HIGH && lastTouchState == LOW) {
        if (currentServoAngle == SERVO_MIN_ANGLE) {
            // Move servo to max position
            moveServoSmooth(SERVO_MAX_ANGLE);
            digitalWrite(LED_PIN, HIGH);
        } else {
            // Move servo to min position
            moveServoSmooth(SERVO_MIN_ANGLE);
            digitalWrite(LED_PIN, LOW);
        }
        
        // Generate a new random index each time
        int randomIndex = random(0, messageCount);
        Serial.print("Selected message index: ");
        Serial.println(randomIndex);
        
        // Get a random message (not static)
        String message = messages[randomIndex];
        
        // Show scrolling text that enters from right and scrolls off to the left
        showScrollingText(display, message, 50);
        
        // Wait for text scrolling to complete
        while (isAnimating()) {
            updateAnimation(display);
            delay(1);
        }
        
        // After text scrolls, show dancing couple animation
        startAnimation(drawDancingCouple, "frame1", "frame2", 10, 250, display);
    }

    lastTouchState = currentTouchState;

    // If no animation is running, start the continuous lady and gentleman animation
    if (!isAnimating()) {
        startAnimation(drawLadyAndGentleman, "small", "big", 100, 500, display);  // 100 iterations, will restart when done
    }

    // Update animation (non-blocking)
    updateAnimation(display);
}

