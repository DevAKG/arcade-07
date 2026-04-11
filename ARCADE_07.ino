#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN 13
#define DATA_PIN 11
#define CS_PIN 10

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Pins
const int potmeter = A3;
const int blue = 12;
const int green = A4;

// Game State Management
int gameState = -1; // -1: Boot, 0: Menu, 1: Snake, 2: Space Defender
int gameChoice = 1; 
int points = 0;
int record = 0;

// Shared Joystick
int prev;

// --- CUSTOM 3x5 FONT ---
const byte font[10][5] = {
  {0b111, 0b101, 0b101, 0b101, 0b111}, // 0
  {0b010, 0b110, 0b010, 0b010, 0b111}, // 1
  {0b111, 0b001, 0b111, 0b100, 0b111}, // 2
  {0b111, 0b001, 0b111, 0b001, 0b111}, // 3
  {0b101, 0b101, 0b111, 0b001, 0b001}, // 4
  {0b111, 0b100, 0b111, 0b001, 0b111}, // 5
  {0b111, 0b100, 0b111, 0b101, 0b111}, // 6
  {0b111, 0b001, 0b010, 0b010, 0b010}, // 7
  {0b111, 0b101, 0b111, 0b101, 0b111}, // 8
  {0b111, 0b101, 0b111, 0b001, 0b111}  // 9
};

// --- SNAKE VARIABLES ---
int coord[100][2];
int leng = 1;
int snakeDots[5][2] {{10,5},{3,1},{25,3},{15,7},{11,3}};

// --- SPACE DEFENDER VARIABLES ---
int shipY = 4;
int bulletX = -1, bulletY = -1;
int enemyX = 31, enemyY = 4;

void led(int x, int y) { mx.setPoint(y-1, x-1, 1); }

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
  int numDigits = (num >= 100) ? 3 : (num >= 10) ? 2 : 1;
  int currentX = startX + (numDigits - 1) * 4; 
  while (temp > 0) {
    drawDigit(temp % 10, currentX);
    currentX -= 4;
    temp /= 10;
  }
}

void handleGameOver() {
  if (points > record) record = points;
  mx.clear();
  // Draw "GO" (Game Over)
  led(2,2); led(2,3); led(2,4); led(3,1); led(4,1); led(5,2); led(5,3); led(5,4); led(4,3); led(5,3); // G
  led(10,2); led(10,3); led(10,4); led(11,1); led(11,5); led(12,1); led(12,5); led(13,2); led(13,3); led(13,4); // O
  
  drawNumber(points, 20); // Show score on the right
  delay(3000); 
  gameState = 0; // Return to menu
}

int getJoystick() {
  int x = analogRead(A0); int y = analogRead(A1); int sw = analogRead(A2);
  if(y < 300) { prev = 0; return 0; } 
  if(y > 700) { prev = 1; return 1; } 
  if(x > 700) { prev = 2; return 2; } 
  if(x < 300) { prev = 3; return 3; } 
  if(sw < 50) { return 5; }           
  return prev;
}

void setup() {
  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 2);
  pinMode(blue, OUTPUT); pinMode(green, OUTPUT);
  randomSeed(analogRead(A5));
}

void loop() {
  if (gameState == -1) { // BOOT
    mx.clear();
    drawA(5); draw0(13); draw7(21);
    delay(2000);
    gameState = 0;
  }

  if (gameState == 0) { // MENU
    mx.clear();
    int joy = getJoystick();
    if (joy == 2) gameChoice = 2; 
    if (joy == 3) gameChoice = 1; 
    
    if (gameChoice == 1) { drawS(5); led(15, 4); led(16, 4); } 
    else { drawD(22); led(18, 4); led(17, 4); }

    if (getJoystick() == 5) {
      points = 0; leng = 1;
      coord[0][0] = 5; coord[0][1] = 4;
      enemyX = 31; bulletX = -1;
      gameState = gameChoice;
      delay(300);
    }
    delay(100);
  }

  if (gameState == 1) runSnake();
  if (gameState == 2) runSpaceDefender();
}

void runSnake() {
  int speed = map(analogRead(potmeter), 0, 1023, 300, 50);
  delay(speed);
  for(int x = leng; x > 0; x--) { coord[x][0] = coord[x-1][0]; coord[x][1] = coord[x-1][1]; }
  int joy = getJoystick();
  if(joy == 0) coord[0][1] = coord[1][1]+1;
  if(joy == 1) coord[0][1] = coord[1][1]-1;
  if(joy == 2) coord[0][0] = coord[1][0]+1;
  if(joy == 3) coord[0][0] = coord[1][0]-1;

  for (int i = 0; i < 5; i++) {
    if (coord[0][0] == snakeDots[i][0] && coord[0][1] == snakeDots[i][1]) {
      leng++; points += 5;
      snakeDots[i][0] = random(1, 32); snakeDots[i][1] = random(1, 8);
    }
  }
  if (coord[0][0] < 1 || coord[0][0] > 32 || coord[0][1] < 1 || coord[0][1] > 8) {
    handleGameOver();
    return;
  }
  mx.clear();
  for (int i = 0; i < leng; i++) led(coord[i][0], coord[i][1]);
  for (int i = 0; i < 5; i++) led(snakeDots[i][0], snakeDots[i][1]);
}

void runSpaceDefender() {
  int speed = map(analogRead(potmeter), 0, 1023, 100, 20);
  delay(speed);
  mx.clear();
  int joy = getJoystick();
  if (joy == 0 && shipY < 8) shipY++;
  if (joy == 1 && shipY > 1) shipY--;
  if (joy == 5 && bulletX == -1) { bulletX = 3; bulletY = shipY; }
  if (bulletX != -1) { led(bulletX, bulletY); bulletX += 2; if (bulletX > 32) bulletX = -1; }
  led(enemyX, enemyY);
  enemyX--;
  if (enemyX < 1) { handleGameOver(); return; }
  if (bulletX >= enemyX && bulletY == enemyY) {
    points++; bulletX = -1; enemyX = 31; enemyY = random(1, 9);
    digitalWrite(green, 1); delay(10); digitalWrite(green, 0);
  }
  led(1, shipY); led(2, shipY); led(2, shipY+1); led(2, shipY-1); 
}

// Menu Helper Graphics
void drawA(int x) { led(x,2); led(x,3); led(x,4); led(x,5); led(x+1,1); led(x+2,1); led(x+3,2); led(x+3,3); led(x+3,4); led(x+3,5); led(x+1,3); led(x+2,3); }
void draw0(int x) { for(int i=1;i<6;i++){ led(x,i); led(x+3,i); } led(x+1,1); led(x+2,1); led(x+1,5); led(x+2,5); }
void draw7(int x) { for(int i=1;i<5;i++) led(x+i-1, 5); for(int i=1;i<5;i++) led(x+4-i, i); }
void drawS(int x) { led(x,1); led(x+1,1); led(x+2,1); led(x,2); led(x,3); led(x+1,3); led(x+2,3); led(x+2,4); led(x,5); led(x+1,5); led(x+2,5); }
void drawD(int x) { for(int i=1;i<6;i++) led(x,i); led(x+1,1); led(x+2,2); led(x+2,3); led(x+2,4); led(x+1,5); }