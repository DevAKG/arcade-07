#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN 13
#define DATA_PIN 11
#define CS_PIN 10

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// ==========================================
// FONT ARRAY (5 rows x 3 cols, digits 0–9)
// Each row is a 3-bit bitmask (bits 2,1,0 = left,mid,right)
// ==========================================
const byte font[10][5] = {
  {0b111, 0b101, 0b101, 0b101, 0b111}, // 0
  {0b010, 0b110, 0b010, 0b010, 0b111}, // 1
  {0b111, 0b001, 0b111, 0b100, 0b111}, // 2
  {0b111, 0b001, 0b111, 0b001, 0b111}, // 3
  {0b101, 0b101, 0b111, 0b001, 0b001}, // 4
  {0b111, 0b100, 0b111, 0b001, 0b111}, // 5
  {0b111, 0b100, 0b111, 0b101, 0b111}, // 6
  {0b111, 0b001, 0b001, 0b001, 0b001}, // 7
  {0b111, 0b101, 0b111, 0b101, 0b111}, // 8
  {0b111, 0b101, 0b111, 0b001, 0b111}, // 9
};

// Pins
const int potmeter = A3;
const int green = A4;
const int blue = 12;
const int gameOverLed = 9;

// Game State
int gameState = -1;
int gameChoice = 1;
int points = 0;
int record = 0;
int snakeDir = 2;

// Shared Game Variables
int coord[100][2];
int leng = 1;
int snakeDots[5][2] {{10,5},{3,1},{25,3},{15,7},{11,3}};
int shipY = 4;
int bulletX = -1, bulletY = -1;
int enemyX = 31, enemyY = 4;
float birdY = 4.0;
float birdVelocity = 0.0;
int pipeX = 31;
int pipeGap = 4;

// Sword/Fighting Variables
int pX = 5, eX = 28, pHP = 3, eHP = 3, enemyLevel = 1;
unsigned long lastAttack = 0;

void led(int x, int y) { mx.setPoint(y-1, x-1, 1); }

void flashGreen() {
  digitalWrite(green, HIGH);
  delay(30);
  digitalWrite(green, LOW);
}

void flashRed() {
  digitalWrite(blue, HIGH);
  delay(30);
  digitalWrite(blue, LOW);
}

void handleGameOver() {
  if (points > record) record = points;
  digitalWrite(blue, HIGH); // Red ON during game over screen
  mx.clear();
  led(2,2); led(2,3); led(2,4); led(3,1); led(4,1); led(5,2); led(5,3); led(5,4); led(4,3);
  led(10,2); led(10,3); led(10,4); led(11,1); led(11,5); led(12,1); led(12,5); led(13,2); led(13,3); led(13,4);
  drawNumber(points, 20);
  delay(3000);
  digitalWrite(blue, LOW); // Red OFF after game over screen
  gameState = 0;
}

int getJoystick() {
  int x = analogRead(A0); int y = analogRead(A1); int sw = analogRead(A2);
  if(y < 300) return 1;
  if(y > 700) return 0;
  if(x > 700) return 2;
  if(x < 300) return 3;
  if(sw < 50) return 5;
  return -1;
}

void setup() {
  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 2);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(gameOverLed, OUTPUT);
  digitalWrite(gameOverLed, LOW);
  randomSeed(analogRead(A5));
}

void loop() {
  if (gameState == -1) {
    mx.clear();
    drawA(5); draw0(13); draw7(21);
    delay(2000);
    gameState = 0;
  }

  if (gameState == 0) {
    mx.clear();
    int joy = getJoystick();
    if (joy == 2) { gameChoice++; if (gameChoice > 4) gameChoice = 1; delay(250); }
    if (joy == 3) { gameChoice--; if (gameChoice < 1) gameChoice = 4; delay(250); }
    led(9,3); led(10,2); led(10,4);
    led(23,3); led(22,2); led(22,4);

    if (gameChoice == 1) drawS(15);
    if (gameChoice == 2) drawSpaceIcon(15);
    if (gameChoice == 3) drawK(15);
    if (gameChoice == 4) drawF(15);

    if (getJoystick() == 5) {
      while(getJoystick() == 5);
      points = 0;
      leng = 1;
      coord[0][0] = 5; coord[0][1] = 4;
      coord[1][0] = 4; coord[1][1] = 4;
      snakeDir = 2;
      birdY = 4.0; birdVelocity = 0; pipeX = 31;
      pX = 5; eX = 28; pHP = 3; eHP = 3; enemyLevel = 1;
      mx.clear();
      gameState = gameChoice;
      delay(300);
    }
    delay(100);
  }

  if (gameState == 1) runSnake();
  if (gameState == 2) runSpaceDefender();
  if (gameState == 3) runSwordFight();
  if (gameState == 4) runFlappy();
}

void runSnake() {
  int speed = map(analogRead(potmeter), 0, 1023, 300, 50);
  delay(speed);
  for(int x = leng; x > 0; x--) {
    coord[x][0] = coord[x-1][0];
    coord[x][1] = coord[x-1][1];
  }
  int joy = getJoystick();
  if (joy >= 0 && joy <= 3) snakeDir = joy;
  if(snakeDir == 0) coord[0][1]--;
  if(snakeDir == 1) coord[0][1]++;
  if(snakeDir == 2) coord[0][0]++;
  if(snakeDir == 3) coord[0][0]--;
  if (coord[0][0] < 1 || coord[0][0] > 32 || coord[0][1] < 1 || coord[0][1] > 8) {
    handleGameOver(); return;
  }
  for (int i = 0; i < 5; i++) {
    if (coord[0][0] == snakeDots[i][0] && coord[0][1] == snakeDots[i][1]) {
      leng++; points += 5;
      flashGreen();
      snakeDots[i][0] = random(1, 32);
      snakeDots[i][1] = random(1, 8);
    }
  }
  mx.clear();
  for (int i = 0; i < leng; i++) led(coord[i][0], coord[i][1]);
  for (int i = 0; i < 5; i++) led(snakeDots[i][0], snakeDots[i][1]);
}

void runSwordFight() {
  int speed = max(20, 100 - (enemyLevel * 5));
  delay(speed);
  mx.clear();
  int joy = getJoystick();
  if (joy == 2 && pX < eX - 2) pX++;
  if (joy == 3 && pX > 1) pX--;
  if (joy == 5 && millis() - lastAttack > 400) {
    lastAttack = millis();
    led(pX + 1, 4); led(pX + 2, 4); led(pX + 3, 4);
    if (abs(pX - eX) <= 4) {
      eHP--; points += 10;
      flashGreen();
    }
  }
  if (random(0, 10) > 3) { if (eX > pX + 2) eX--; else if (eX < pX + 2) eX++; }
      if (abs(pX - eX) <= 3 && random(0, 15) == 1) { pHP--; flashRed(); } // Flash red on hit
  if (pHP <= 0) { handleGameOver(); return; }
  if (eHP <= 0) { points += 50; enemyLevel++; eHP = 2 + (enemyLevel / 2); eX = 31; flashGreen(); }
  led(pX, 3); led(pX, 4); led(pX, 5); led(pX-1, 6); led(pX+1, 6);
  led(eX, 3); led(eX, 4); led(eX, 5); led(eX-1, 6); led(eX+1, 6);
  for(int i=0; i<pHP; i++) led(1+i, 1);
  for(int i=0; i<eHP; i++) led(32-i, 1);
}

void runFlappy() {
  int speed = map(analogRead(potmeter), 0, 1023, 120, 40);
  delay(speed);
  mx.clear();
  int joy = getJoystick();
  if (joy == 5 || joy == 0) birdVelocity = -1.2;
  birdVelocity += 0.3; birdY += birdVelocity;
  pipeX--;
  if (pipeX < 1) { pipeX = 31; pipeGap = random(2, 6); points++; flashGreen(); }
  int bY = round(birdY);
  if (bY < 1 || bY > 8) { handleGameOver(); return; }
  if (pipeX == 4 || pipeX == 3) { if (bY < pipeGap || bY > pipeGap + 2) { handleGameOver(); return; } }
  for (int y = 1; y <= 8; y++) { if (y < pipeGap || y > pipeGap + 2) { led(pipeX, y); led(pipeX+1, y); } }
  led(4, bY);
}

void runSpaceDefender() {
  int speed = map(analogRead(potmeter), 0, 1023, 80, 20);
  delay(speed);
  mx.clear();
  int joy = getJoystick();
  if (joy == 0 && shipY < 7) shipY++;
  if (joy == 1 && shipY > 2) shipY--;
  if (joy == 5 && bulletX == -1) { bulletX = 3; bulletY = shipY; }
  if (bulletX != -1) { led(bulletX, bulletY); bulletX += 2; if (bulletX > 32) bulletX = -1; }
  led(enemyX, enemyY); enemyX--;
  if (enemyX < 1) { handleGameOver(); return; }
  if (bulletX >= enemyX && (bulletY == enemyY)) { points++; bulletX = -1; enemyX = 31; enemyY = random(1, 9); flashGreen(); }
  led(1, shipY); led(2, shipY); led(2, shipY+1); led(2, shipY-1);
}

// --- Menu Icons & Letters ---
void drawA(int x) { led(x,2); led(x,3); led(x,4); led(x,5); led(x+1,1); led(x+2,1); led(x+3,2); led(x+3,3); led(x+3,4); led(x+3,5); led(x+1,3); led(x+2,3); }
void draw0(int x) { for(int i=1;i<6;i++){ led(x,i); led(x+3,i); } led(x+1,1); led(x+2,1); led(x+1,5); led(x+2,5); }
void draw7(int x) { for(int i=1;i<5;i++) led(x+i-1, 5); for(int i=1;i<5;i++) led(x+4-i, i); }
void drawS(int x) { led(x,1); led(x+1,1); led(x+2,1); led(x,2); led(x,3); led(x+1,3); led(x+2,3); led(x+2,4); led(x,5); led(x+1,5); led(x+2,5); }
void drawSpaceIcon(int x) { led(x,4); led(x+1,3); led(x+1,4); led(x+1,5); led(x+2,4); led(x+3,4); }
void drawK(int x) { for(int i=1;i<6;i++) led(x,i); led(x+1,3); led(x+2,2); led(x+2,4); led(x+3,1); led(x+3,5); }
void drawF(int x) { for(int i=1;i<6;i++) led(x,i); led(x+1,1); led(x+2,1); led(x+1,3); led(x+2,3); }

// --- Number Rendering ---
void drawDigit(int digit, int startX) {
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 3; x++) {
      if (bitRead(font[digit][y], 2 - x)) led(startX + x, y + 2);
    }
  }
}

void drawNumber(int num, int startX) {
  if (num == 0) { drawDigit(0, startX); return; }
  int temp = num;
  int numDigits = (num >= 10) ? 1 : 0;
  int currentX = startX + numDigits * 4;
  while (temp > 0) {
    drawDigit(temp % 10, currentX);
    currentX -= 4;
    temp /= 10;
  }
}