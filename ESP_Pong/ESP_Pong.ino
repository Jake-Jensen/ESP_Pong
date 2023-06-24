#include "Arduino.h"
#include <Wire.h>  
#include "HT_SSD1306Wire.h"

// Playfield is 128x64
SSD1306Wire  factory_display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst

const int PlaySpeed = 20; // milliseconds between frame updates
const int PlayerHeight = 15;
const int PlayerWidth = 4;
const int BallSize = 2;
const int BallSpeed = 4; // Pixels per second?
const int OLED_WIDTH = 128;
const int OLED_HEIGHT = 64;

const int PLAYER_ONE = 1;
const int PLAYER_TWO = 2;
const int PLAYER_BALL = 3;

int Player1_X = 0 + PlayerWidth;
int Player1_Y = 0;
int Player2_X = 128 - PlayerWidth;
int Player2_Y = 0;
int BallX = 0;
int BallY = 0;
int BallDestinationX = 0;
int BallDestinationY = 0;

const bool IsDebug = true;

void LOG(String Message) {
  if (IsDebug) {
    Serial.println(String("[LOG]: " + Message));
  }
}

void VextON(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
  
}

void VextOFF(void) //Vext default OFF
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}

void DrawPlayers(int Player, int X_Pos, int Y_Pos) {
  switch (Player) {
  case PLAYER_ONE:
    // Left player
    factory_display.drawVerticalLine(0 + PlayerWidth, Y_Pos, PlayerHeight);
    break;
  case PLAYER_TWO:
    // Right player
    factory_display.drawVerticalLine(OLED_WIDTH - PlayerWidth, Y_Pos, PlayerHeight);
    break;
  case PLAYER_BALL:
    // The ball
    factory_display.drawCircle(X_Pos, Y_Pos, BallSize);
    break;
  default:
    break;
  }
  
}

bool IsMovingLeft = false;
bool IsMovingUp = false;

void SendBallRandomInit() {
  IsMovingLeft = random(1);
  IsMovingUp = random(1);
}

void UpdateBallPosition() {
  if (BallX <= 1) {
    IsMovingLeft = false;
  }
  if (BallX >= OLED_WIDTH - 1) {
    IsMovingLeft = true;
  }

  if (BallY >= OLED_HEIGHT - 1) {
    IsMovingUp = false;
  }

  if (BallY <= 1) {
    IsMovingUp = true;
  }

  if (IsMovingLeft) {
    BallX--;
  } else {
    BallX++;
  }

  if (IsMovingUp) {
    BallY++;
  } else {
    BallY--;
  }
}

int Target1 = 0;
int Target2 = 0;
// This will interp the paddle towards the ball's final trajectory so it can hit it
void GenerateSpecificMovement(int Player) {
  if (Player == PLAYER_ONE) {
    if (Player1_Y < BallY) {
      Player1_Y++;
    }
    if (Player1_Y > BallY) {
      Player1_Y--;
    }
  }

  if (Player == PLAYER_TWO) {
    if (Player2_Y < BallY) {
      Player2_Y++;
    }
    if (Player2_Y > BallY) {
      Player2_Y--;
    }
  }
}

void SetTargets(int Player) {
  int PriorTarget1 = Target1;
  int PriorTarget2 = Target2;

  if (Player == PLAYER_ONE) {
    Target1 = random(OLED_HEIGHT);
    if ((Target1 - (PriorTarget1* -1)) < 5) {
      if (Target1 >= OLED_HEIGHT / 2) {
        Target1 = Target1 + 5;
      } else {
        Target1 = Target1 - 5;
      }
    }
    LOG("Set target for player 1 to: " + String(Target1));
  } else {
    Target2 = random(OLED_HEIGHT);
    if ((Target2 - (PriorTarget2 * -1)) < 5) {
      if (Target2 >= OLED_HEIGHT / 2) {
        Target2 = Target2 + 5;
      } else {
        Target2 = Target2 - 5;
      }
    }
    LOG("Set target for player 2 to: " + String(Target2));
  }  
}

// This will move the paddle when ball is moving away from it // DEBUG! Just moves them both randomly
void GenerateRandomMovement() {
  // If Player1's location - their height is above the OLED bounds, set their position the other way
  if (Player1_Y == Target1) {
    SetTargets(PLAYER_ONE);
    if (Target1 > OLED_HEIGHT) {
      Target1 == OLED_HEIGHT - PlayerHeight;
    }
    if (Target1 < OLED_HEIGHT) {
      Target1 == OLED_HEIGHT + PlayerHeight;
    }
  } else {
    if (Player1_Y > Target1) {
      Player1_Y--;
    } else {
      Player1_Y++;
    }
  }

  if (Player2_Y == Target2) {
    SetTargets(PLAYER_TWO);
      if (Target2 > OLED_HEIGHT) {
        Target2 == OLED_HEIGHT - PlayerHeight;
      }
      if (Target2 < OLED_HEIGHT) {
        Target2 == OLED_HEIGHT + PlayerHeight;
      }
  } else {
    if (Player2_Y > Target2) {
      Player2_Y--;
    } else {
      Player2_Y++;
    }
  }

}

void setup() {
  // Enable display, disable Lora, enable serial
  Serial.begin(115200);
	VextON();
	delay(100);
	factory_display.init();
  LOG("Everything initialized, clearing screen.");
  factory_display.clear();
  factory_display.drawString(0, 0, "Initializing factory_display.");
  factory_display.display();
  LOG("Displaying the init message.");
  delay(1000);
  factory_display.clear();
  LOG("Going to loop mode.");
  pinMode(LED ,OUTPUT);
	digitalWrite(LED, LOW);  
  LOG("Drawing DEBUG");
  factory_display.drawString(0, 0, "DEBUG");
  factory_display.drawString((128 / 2) - 4, 0, "DEBUG");
  factory_display.display();
  delay(1000);

  factory_display.clear();
  LOG("Drawing players.");
  DrawPlayers(PLAYER_ONE, Player1_X, Player1_Y);
  DrawPlayers(PLAYER_TWO, Player2_X, Player2_Y);
  DrawPlayers(PLAYER_BALL, (OLED_WIDTH / 2) - (BallSize / 2), (OLED_HEIGHT / 2));
  LOG("Updating factory_display.");
  factory_display.display();
  delay(PlaySpeed);
  SendBallRandomInit();
  SetTargets(PLAYER_ONE);
  SetTargets(PLAYER_TWO);
}

void loop() {
  // LOG("Drawing players.");
  // First, update ball position. This should be an interp from CurrentPos to DestPos
  // Not drawing ball yet, problem
  
  UpdateBallPosition();

  GenerateRandomMovement();

  if (BallX <= OLED_WIDTH / 2) {
    GenerateSpecificMovement(PLAYER_ONE);
  } else {
    GenerateSpecificMovement(PLAYER_TWO);
  }
  
  DrawPlayers(PLAYER_ONE, Player1_X, Player1_Y);
  DrawPlayers(PLAYER_TWO, Player2_X, Player2_Y);
  DrawPlayers(PLAYER_BALL, BallX, BallY);
  

  // LOG("Updating factory_display.");
  factory_display.display();
  delay(PlaySpeed);
  factory_display.clear();

}
