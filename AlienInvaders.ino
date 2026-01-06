/*
======================================= ALIEN INVADERS =======================================
This is the classic Alien Invaders arcade-style game, where the goal is to survive the
Alien Invasion for as long as possible. Aliens are constantly coming in and advancing
towards the player, who must dodge their bullets and shoot the Aliens to get points.
Whenever the player is hit by an Alien bullet, they lose a life, and when an Alien reaches
the player, the player loses a life. When the player runs out of lives, the game is over.

Controls:
Left Button   = Move left
Right Button  = Move right
Up Button     = Shoot
===============================================================================================
*/

/*
Left off: wave cooldown not working, bullets shooting from all aliens and don't move correctly, running
out of lives doesn't end game, can't shoot or hit aliens that touch you, need to display score at the end
and keep track of high score in hard drive
*/


// ==================================== VARIABLES ==================================== //

// CONSTANTS
const int pointsPerHit = 1; // How many points are given for each Alien hit
const int startLives = 5;  // How many lives the Player starts out with

const int buttonDebounce = 150; // How long to wait before button can be pressed again (ms)
int waveCooldown = 3000; // How long to wait before button can be pressed again (ms)
const int pShootCooldown = 500; // How long to wait before you can shoot again (ms)
int aShootCooldown = 5000; // How long to wait before an Alien can shoot again (ms)
const int bulletSpeed = 300; // How long it takes for a bullet to travel one pixel forward (ms)
int alienSpeed = 3000; // How long it takes for the aliens to move one space (ms)

// Logic
bool canPress[] = {true, true, true}; // Whether or not each Button can be pressed: {Left, Right, Up}
unsigned long lastPressed[] = {buttonDebounce, buttonDebounce, pShootCooldown}; // Time tracker, stores the last time each Button was pressed (ms)
unsigned long lastUpdated[] = {0, 0, 0, waveCooldown}; // Last time each game element was updated: {pBullets, aBullets, aliens, alien shot, wave started}

// Game Data
int score = 0;
int lives = startLives;
int playerX = 3; // Player X-Position (0 - 7)
bool gameOver = false;

int alienMoveStep = 0; // 0 - 3: 0 = down, 1 = right, 2 = down, 3 = left
int wave = 0; // Which wave the game is on; higher wave = more difficult
int aliensLeft = 0; // Keeps track of aliens left in current wave, used to detect when to start new wave
int rowSpawned = 0; // Keep track of which row in the current wave of aliens has been spawned on the screen

byte pixels[8]; // Array of pixels, representing current screen state (8 columns, each one an 8-bit number representing enabled rows)
byte aliens[8]; // Array of all the "aliens" currently on the screen
byte pBullets[8]; // Array of all the "player bullets" currently on the screen
byte aBullets[8]; // Array of all the "alien bullets" currently on the screen

// Pins
const int greenLED = 6;
const int redLED = 5;

const int buttonR = 1;
const int buttonU = 2;
const int buttonD = 3;
const int buttonL = 4;

const int colPins[8] = {
  9, A2, A3, 7, A4, 8, 10, A5
};

const int serialInput = 11;
const int parallelUpdate = 12;
const int serialShiftClock = 13;

// Skull face image
const int skullFace[] = {
  0x0, 0x0A, 0x4A, 0xA4, 0xA4, 0x4A, 0x0A, 0x0
};

const int endTextLen = 19;
const int numCharsLen = 10;

const int endText[endTextLen][5] = {
  {0xFE, 0x82, 0x82, 0x92, 0x9E}, // G
  {0x3E, 0x48, 0x88, 0x48, 0x3E}, // A
  {0x7E, 0x80, 0x78, 0x80, 0x7E}, // M
  {0xFE, 0x92, 0x92, 0x92, 0x92}, // E
  {0x0, 0x0, 0x0, 0x0, 0x0},      // Space
  {0xFE, 0x82, 0x82, 0x82, 0xFE}, // O
  {0xF0, 0x0C, 0x02, 0x0C, 0xF0}, // V
  {0xFE, 0x92, 0x92, 0x92, 0x92}, // E
  {0xFE, 0x90, 0x90, 0x98, 0x66}, // R
  {0x0, 0x0, 0x0, 0x0, 0x0},      // Space
  {0x0, 0x0, 0x0, 0x0, 0x0},      // Space
  {0x62, 0x92, 0x92, 0x92, 0x8C}, // S
  {0x7C, 0x82, 0x82, 0x82, 0x44}, // C
  {0xFE, 0x82, 0x82, 0x82, 0xFE}, // O
  {0xFE, 0x90, 0x90, 0x98, 0x66}, // R
  {0xFE, 0x92, 0x92, 0x92, 0x92}, // E
  {0x0, 0x44, 0x0, 0x0, 0x0},     // Colon
  {0x0, 0x0, 0x0, 0x0, 0x0}       // Space
};

const int numChars[numCharsLen][5] = {
  {0x7C, 0x8A, 0x92, 0xA2, 0x7C},
  {0x0, 0x42, 0xFE, 0x02, 0x0},
  {0x42, 0x86, 0x8A, 0x92, 0x62},
  {0x44, 0x82, 0x92, 0x92, 0x6C},
  {0x18, 0x28, 0x48, 0xFE, 0x08},
  {0xE4, 0x92, 0x92, 0x92, 0x8C},
  {0x1C, 0x32, 0x52, 0x92, 0x0C},
  {0x80, 0x80, 0x8E, 0xB0, 0xC0},
  {0x66, 0x92, 0x92, 0x92, 0x66},
  {0x60, 0x92, 0x94, 0x98, 0x70}
};

// const int numChars[numCharsLen][5] = {
//   &char0, &char1, &char2, &char3, &char4, &char5, &char6, &char7, &char8, &char9
// };

// ==================================== GAME MECHANICS ==================================== //

// ------------------------------------ Buttons ------------------------------------

void leftButtonPressed() {
  playerX = fmax(0, playerX - 1);
  Serial.println("Left button pressed");
}

void rightButtonPressed() {
  playerX = fmin(7, playerX + 1);
  Serial.println("Right button pressed");
}

void upButtonPressed() {
  shootBullet();
  Serial.println("Up button pressed");
}

void (*buttonPresses[])() = {
  leftButtonPressed,
  rightButtonPressed,
  upButtonPressed
};

// ------------------------------------ Core ------------------------------------

// Update Game Data based on time, like moving aliens and bullets
void updateData() {
  updateBullets();
  updateAliens();
}

// Detect button presses and update Game Data, like Player Position and shooting Player Bullets
void buttonBindings() {
  // Get current button and time readings
  bool bL = digitalRead(buttonL) == LOW;
  bool bR = digitalRead(buttonR) == LOW;
  bool bU = digitalRead(buttonU) == LOW;
  int signal[] = {bL, bR, bU};
  unsigned long t = millis();

  // Update Left and Right canPress flags based on current time
  for (int i = 0; i < 2; i++) {
    if (t - lastPressed[i] >= buttonDebounce) {
      canPress[i] = true;
    } else {
      canPress[i] = false;
    }
  }

  // Update Shoot Button canPress flag
  if (t - lastPressed[2] >= pShootCooldown) {
    canPress[2] = true;
  } else {
    canPress[2] = false;
  }

  // Detect if each Button is pressed, calling the correct ___ButtonPressed() function
  for (int i = 0; i < 3; i++) {
    if (canPress[i] && signal[i]) {
      canPress[i] = false;
      lastPressed[i] = t;
      (*buttonPresses[i])();
    }
  }

}

// Update the pixels[] array using current Game Data
void updatePixels() {
  for (int i = 0; i < 8; i++) {
    pixels[i] = (aliens[i] | pBullets[i] | aBullets[i]);
    pixels[playerX] = pixels[playerX] | 0x01;
  }
}

// ------------------------------------ Mechanics ------------------------------------

void shootBullet() {
  // Add an additional 1 at the 2nd bit in the player's column in pBullets[] array
  pBullets[playerX] = pBullets[playerX] | 0x02;
}

void updateBullets() {
  // Check if it's time to update bullets, then shift all bullets forward in each column, checking for collisions

  // Player Bullets
  unsigned long t = millis();
  if (t - lastUpdated[0] >= bulletSpeed) { // Check if it's time to update
    lastUpdated[0] = t;

    for (int i = 0; i < 8; i++) { // Loop through each column

      pBullets[i] <<= 1; // Move all pBullets in this column forward
      // This number will have a 1 in the bit where the bullet collides with Alien
      int collisionA = pBullets[i] & aliens[i];
      // This number will have a 1 in the bit where the pBullet collides with an aBullet
      int collisionB = pBullets[i] & aBullets[i];

      if (collisionA > 0) {
        // Alien-Bullet collision was detected; increase Score, remove the hit Alien and the Bullet which hit it
        score += pointsPerHit;
        aliensLeft -= 1;
        aliens[i] = aliens[i] - collisionA;
        pBullets[i] = pBullets[i] - collisionA;
      }

      if (collisionB > 0) {
        // Bullet-Bullet collision (1) was detected; remove both bullets
        pBullets[i] = pBullets[i] - collisionB;
        aBullets[i] = aBullets[i] - collisionB;
      }

      aBullets[i] >>= 1; // Move all aBullets in this column backward
      collisionB = pBullets[i] & aBullets[i];
      
      if (aBullets[i] & 0x01 && playerX == i) {
        // Bullet-Player collision was detected; remove the Bullet and subtract 1 life
        lives -= 1;
        aBullets[i] = aBullets[i] - 0x01;
      }

      if (collisionB > 0) {
        // Bullet-Bullet collision (2) was detected; remove both bullets
        pBullets[i] = pBullets[i] - collisionB;
        aBullets[i] = aBullets[i] - collisionB;
      }
    }
  }
}

void updateAliens() {
  checkWave();

  unsigned long t = millis();
  if (t - lastUpdated[3] >= waveCooldown) {

    // Reset lastUpdated[] values if new wave just spawned
    if (rowSpawned == 0) {
      lastUpdated[1] = t - alienSpeed;
      lastUpdated[2] = t;
    }

    // Check if it's time to update alien positions, then move aliens
    if (t - lastUpdated[1] >= alienSpeed) {
      moveAliens();
      lastUpdated[1] = t;
    }

    // Check if it's time for an Alien to shoot, then shoot
    if (t - lastUpdated[2] >= aShootCooldown) {
      alienShot();
      lastUpdated[2] = t;
    }
  }
}

// ------------------------------------ Aliens ------------------------------------

void moveAliens() {
  if (alienMoveStep == 0 || alienMoveStep == 2) {
    moveAliensDown();
  } else if (alienMoveStep == 1) {
    switchAliensSide();
  } else if (alienMoveStep == 3) {
    switchAliensSide();
  }

  if (alienMoveStep == 3) {
    alienMoveStep = 0;
  } else {
    alienMoveStep++;
  }

  for (int i = 0; i < 8; i++) {
    // Check again for Alien-Bullet collisions after alien move
    int collisionA = aliens[i] & pBullets[i];
    if (collisionA > 0) {
      score += pointsPerHit;
      aliensLeft -= 1;
      aliens[i] = aliens[i] - collisionA;
      pBullets[i] = pBullets[i] - collisionA;
    }
  }
}

void alienShot() {
  // Check if can shoot, then choose random alien to shoot from
  if (wave > 0 && aliensLeft > 0 && rowSpawned > 0) {
    int aliensOnScreen = (4*rowSpawned)-(12-aliensLeft);
    long randAlien = random(0, aliensOnScreen);
    int count = -1;
    for (int i = 0; i < 8; i++) {
      byte col = aliens[i];
      for (int j = 0; j < 8; j++) {
        if (col & 0x01 > 0) {
          count++;
          if (count >= randAlien) {
            // Shoot from this Alien: spawn bullet in front of it
            aBullets[i] = aBullets[i] | (0x80 >> (7-j)+1);
            return;
          }
        }
        col >>= 1;
      }
    }
  }
}

void spawnAliensRow() {
  rowSpawned++;
  for (int i = 0; i < 8; i++) {
    if (i % 2 == 0) {
      aliens[i] = aliens[i] | 0x80;
    }
  }
}

void switchAliensSide() {
  for(int i = 0; i < 8; i += 2) {
    byte temp = aliens[i];
    aliens[i] = aliens[i+1];
    aliens[i+1] = temp;
  }
}

// void moveAliensRight() {
//   for (int i = 6; i >= 0; i--) {
//     aliens[i+1] = aliens[i];
//   }
//   aliens[0] = 0x0;
// }

// void moveAliensLeft() {
//   for (int i = 1; i < 8; i++) {
//     aliens[i-1] = aliens[i];
//   }
//   aliens[7] = 0x0;
// }

void moveAliensDown() {
  // Check if any aliens are at the last row; if so, subtract life. Then move all aliens down
  for (int i = 0; i < 8; i++) {
    if (aliens[i] & 0x01 > 0) {
      lives -= 1;
      aliensLeft -= 1;
    }
    aliens[i] >>= 1;
  }

  // If not all the rows of aliens have spawned yet, spawn another row
  if (rowSpawned < 3) {
    spawnAliensRow();
  }
}

void checkWave() {
  // If no aliens left, go to the next wave
  if (aliensLeft <= 0) {
    nextWave();
  }
}

void nextWave() {
  lastUpdated[3] = millis();
  wave++;
  //aliens = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
  alienMoveStep = 0;
  aliensLeft = 12;
  rowSpawned = 0;

  alienSpeed = fmax(100, alienSpeed - 400);
  waveCooldown = fmax(250, waveCooldown - 400);
}

// ==================================== DISPLAY CODE ==================================== //

// Essential
void displayPixels() {

  // "cols" is a one-byte variable keeping track of the currently enabled column
  int cols = 0x01; // 0x01 = 00000001, meaning first column is enabled

  // Display each column of the given array for one millisecond
  for (int i = 0; i < 8; i++) { // Display the corresponding data for each column using the given columns[] array
    enableColumns(cols);        // Select this column
    enableRows(pixels[i]);      // Display the data for this column
    delay(1);                   // Display each column for a millisecond
    enableRows(0x00);           // Clear the data of this column, turning off all the rows, to prepare for next column
    cols <<= 1;                 // Shift "cols" 1 bit to the left, selecting next column
  }
}

void displayPixelsFor(unsigned long ms) {
  ms = ms / 8;
  for (int i = 0; i < ms; i++) {
    displayPixels();
  }
}

void enableColumns(byte value) {
  // Note: To turn on an LED, set column to LOW and row to HIGH

  // Set the columns to LOW that correspond to the binary of "value"
  byte cols = 0x01;
  for (int i = 0; i < 8; i++) {
    digitalWrite(colPins[i], ((value & cols) == cols) ? LOW : HIGH);
    cols <<= 1;
  }
}

void enableRows(int value) {
  // ouput low level to latchPin
  digitalWrite(parallelUpdate, LOW);
  // send serial data to 74HC595
  shiftOut(serialInput, serialShiftClock, LSBFIRST, value);
  // output high level to latchPin, then 74HC595 will update the data to parallel output port
  digitalWrite(parallelUpdate, HIGH);
}

// Higher level
void fillScreen() {
  for (int i = 0; i < 8; i++) {
    pixels[i] = 0xFF;
  }
}

void clearScreen() {
  for (int i = 0; i < 8; i++) {
    pixels[i] = 0x0;
  }
}

void fillRow() {
  for (int i = 0; i < 8; i++) {
    pixels[i] = 0x80;
  }
}

void displayImage(int img[]) {
  for (int i = 0; i < 8; i++) {
    pixels[i] = img[i];
  }
}

// Flashes the LEDs a certain way to indicate startup
void startupLEDs() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, HIGH);
    delay(100);
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, LOW);
    delay(100);
  }
}

void displayScore() {
  int textLen = 160;
  int text[textLen] = {0};

  // Create text[] array that contains all columns to display for the desired end-game message (game over + score)
  int index = 0;
  for (int i = 0; i < endTextLen; i++) {
    for (int j = 0; j < 5; j++) {
      text[index] = endText[i][j];
      index++;
    }
    text[index] = 0x0;
    index++;
  }

  // Add score number chars to text[] array
  int scoreLen = getIntLen(score);
  for (int i = 0; i < scoreLen; i++) {
    int digit = (int)(score / fmax((10 * (scoreLen - i - 1)), 1)) % 10;
    for (int j = 0; j < 5; j++) {
      text[index] = numChars[digit][j];
      index++;
    }
    text[index] = 0x0;
    index++;
  }

  // Display text[] array
  int displayIndex = 0;
  while (displayIndex < textLen - 8) {
    for (int i = 0; i < 8; i++) {
      pixels[i] = text[displayIndex + i];
    }
    displayPixelsFor(100);
    displayIndex++;
  }
  
}

// ==================================== HELPERS ==================================== //

int getIntLen(int n) {
  int digits = 1;
  while (n/10 > 0) {
    digits++;
    n /= 10;
  }
  return digits;
}

// ==================================== STRUCTURE ==================================== //

void setup() {
  // Initialize LED pins
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  // Initialize Button pins
  pinMode(buttonR, INPUT);
  pinMode(buttonU, INPUT);
  pinMode(buttonD, INPUT);
  pinMode(buttonL, INPUT);

  // Initialize 74HC595 pins
  pinMode(serialInput, OUTPUT);
  pinMode(parallelUpdate, OUTPUT);
  pinMode(serialShiftClock, OUTPUT);

  // Initialize grid Column pins
  for (int thisPin = 0; thisPin < 8; thisPin++) {
    pinMode(colPins[thisPin], OUTPUT);
    digitalWrite(colPins[thisPin], HIGH);
  }

  // Initialize pixels array
  for (int i = 0; i < 8; i++) {
    pixels[i] = 0x0;
  }

  startupLEDs();

  Serial.println("STARTED!!!");
}


void loop() {
  if (gameOver) {
    return;
  }

  if (lives > 0) {
    // Update Game Data using pressed buttons and time
    updateData();
    buttonBindings();
    updatePixels();

    // Display the current Game State
    displayPixels();
  } else {
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
    delay(1000);
    for (int i = 0; i < 3; i++) {
      displayImage(skullFace);
      displayPixelsFor(300);
      clearScreen();
      displayPixelsFor(300);
    }
    displayScore();
    gameOver = true;
  }
}


