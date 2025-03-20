#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "images.h"
#include <String.h>
#include <functional>
#include <algorithm> // Added for std::max

#define SCREEN_WIDTH 128  // OLED display width
#define SCREEN_HEIGHT 32  // OLED display height

// Function pointer type for drawing functions
typedef void (*DrawFunction)(Adafruit_SSD1306& display, String state);

// Animation state structure
struct AnimationState {
    bool isAnimating;
    int currentIteration;
    int totalIterations;
    bool isFirstState;
    unsigned long lastFrameTime;
    int delayMs;
    String state1;
    String state2;
    DrawFunction drawFunc;
    // Text scrolling fields
    String scrollText;
    int textX;
    int textWidth;
    // Animation chaining
    std::function<void()> nextAnimation;
};

static AnimationState animState = {false, 0, 0, true, 0, 0, "", "", nullptr, "", 0, 0, nullptr};

// Animation function declarations
void drawLadyAndGentleman(Adafruit_SSD1306& display, String heartsize);
void drawDancingCouple(Adafruit_SSD1306& display, String frame);
void drawScrollText(Adafruit_SSD1306& display, String text);

// Function to show scrolling text (simplified)
inline void showScrollingText(Adafruit_SSD1306& display, String message, int scrollSpeed) {
    // Set up the animation state for custom scrolling
    animState.isAnimating = true;
    animState.currentIteration = 0;
    animState.totalIterations = 1; // Just one iteration
    animState.lastFrameTime = millis();
    animState.delayMs = scrollSpeed; // Smaller values = faster scrolling
    animState.drawFunc = drawScrollText;
    animState.scrollText = message;
    
    // Calculate text width more accurately
    display.setTextSize(2);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(message, 0, 0, &x1, &y1, &w, &h);
    
    // Make sure we get the correct width
    Serial.print("Text width: ");
    Serial.println(w);
    
    // Get a more accurate text width by calculating character by character
    // Each character in size 2 is approximately 12 pixels wide
    int calculatedWidth = message.length() * 12;
    Serial.print("Calculated width: ");
    Serial.println(calculatedWidth);
    
    // Use the larger of the two width calculations plus extra buffer
    int textWidth = (int)w;  // Convert uint16_t to int
    if (calculatedWidth > textWidth) {
        textWidth = calculatedWidth;
    }
    animState.textWidth = textWidth + 20;
    
    // Start text from right edge of screen
    animState.textX = SCREEN_WIDTH;
    
    // Clear any existing nextAnimation
    animState.nextAnimation = nullptr;
}

// Start a regular animation sequence
inline void startAnimation(DrawFunction drawFunc, String state1, String state2, int iterations, int delayMs, Adafruit_SSD1306& display) {
    animState.isAnimating = true;
    animState.currentIteration = 0;
    animState.totalIterations = iterations;
    animState.isFirstState = true;
    animState.lastFrameTime = millis();
    animState.delayMs = delayMs;
    animState.state1 = state1;
    animState.state2 = state2;
    animState.drawFunc = drawFunc;
}

// Non-blocking animation update function
inline void updateAnimation(Adafruit_SSD1306& display) {
    if (!animState.isAnimating) return;

    unsigned long currentTime = millis();
    
    // Handle custom text scrolling
    if (animState.drawFunc == drawScrollText) {
        if (currentTime - animState.lastFrameTime >= animState.delayMs) {
            // Update text position
            animState.textX -= 3; // Scrolling speed
            
            // Check if text scroll is complete (text has completely left the screen)
            if (animState.textX <= -(animState.textWidth + 40)) {  // Add extra buffer space to ensure text completely exits
                // Text has fully scrolled off screen - end animation
                animState.isAnimating = false;
                display.clearDisplay();
                display.display();
                return;
            }
            
            // Draw the text at its current position
            display.clearDisplay();
            drawScrollText(display, animState.scrollText);
            
            animState.lastFrameTime = currentTime;
        }
        return;
    }
    
    // Normal animation updates
    if (currentTime - animState.lastFrameTime >= animState.delayMs) {
        if (animState.currentIteration >= animState.totalIterations) {
            animState.isAnimating = false;
            display.clearDisplay();
            display.display();
            
            if (animState.nextAnimation) {
                auto next = animState.nextAnimation;
                animState.nextAnimation = nullptr;
                next();
            }
            return;
        }

        display.clearDisplay();
        animState.drawFunc(display, animState.isFirstState ? animState.state1 : animState.state2);
        display.display();
        
        if (!animState.isFirstState) {
            animState.currentIteration++;
        }
        animState.isFirstState = !animState.isFirstState;
        
        animState.lastFrameTime = currentTime;
    }
}

// Check if animation is currently running
inline bool isAnimating() {
    return animState.isAnimating;
}

// Stop current animation
inline void stopAnimation(Adafruit_SSD1306& display) {
    animState.isAnimating = false;
    animState.nextAnimation = nullptr;
    display.clearDisplay();
    display.display();
}

// Implementation of animation functions
inline void drawLadyAndGentleman(Adafruit_SSD1306& display, String heartsize) {
    // Draw the lady and gentleman
    display.drawBitmap(96, 8, lady, 9, 16, WHITE);
    display.drawBitmap(114, 8, gentleman, 7, 16, WHITE);

    // Draw the appropriate heart size
    if (heartsize == "small") {
        display.drawBitmap(104, 4, small_heart, 9, 8, WHITE);
    } else if (heartsize == "big") {
        display.drawBitmap(102, 4, big_heart, 13, 11, WHITE);
    }
    display.display();
}

// Dancing couple animation function
inline void drawDancingCouple(Adafruit_SSD1306& display, String frame) {
    if (frame == "frame1") {
        // Frame 1: Position (0,0), size 39x32
        display.drawBitmap(0, 0, dancing_couple_1, 39, 32, WHITE);
    } else if (frame == "frame2") {
        // Frame 2: Position (4,-1), size 31x32
        display.drawBitmap(4, 0, dancing_couple_2, 31, 32, WHITE);
    }
    display.display();
}

// Scrolling text animation function (simplified)
inline void drawScrollText(Adafruit_SSD1306& display, String text) {
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setTextWrap(false);
    
    // Draw text at current X position (from right to left)
    // Center text vertically (typical text height for size 2 is 16 pixels)
    display.setCursor(animState.textX, 9); // Center vertically
    display.print(animState.scrollText);
    display.display();
}

#endif // ANIMATIONS_H