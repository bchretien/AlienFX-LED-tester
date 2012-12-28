#include <iostream>
#include <fstream>

#include "alienfx.h"

extern bool AlienfxNew;

using namespace std;

#define WHITE_COLOR  0xFFFFFF
#define BLACK_COLOR  0x000000
#define BLUE_COLOR   0x00FFFF
#define ON_COLOR     BLUE_COLOR
#define OFF_COLOR    BLACK_COLOR

int main()
{
  ofstream LogFile;
  string desc;
  char input;

  cout << "This program comes without ANY warranty that it is working and/or "
       << "will not damage your hardware or lead to any data loss. It was not "
       << "designed to cause any damage but it might happen.\n\n"
       << "WARNING: Your current AlienFX profile will be erased." << endl;

  input = 0;
  while(input != 'y' && input != 'n'){
    cout << endl << "continue? (y/n)" << endl;
    cin.get(input);
    cin.ignore(100,'\n');
  }
  if(input == 'n')
    return 0;

  LogFile.open("results.log");
  LogFile.width(6);
  LogFile.fill('0');

  cout << "Searching for AlienFX device..." << endl;

  if(!AlienfxInit()){
    cout << "AlienFX device not found. Program terminated" << endl;
    
    #ifdef _WIN32
    system("pause");
    #endif

    return 0;
  }
  else
    cout << "AlienFX device found and connected." << endl << endl;

  if(AlienfxNew){
    LogFile << "New Alienfx Device" << endl;
  }
  else {
    LogFile << "Old Alienfx Device" << endl;
  }

  cout << "Please enter the name of the Alienware PC/laptop you are using" << endl;
  getline(cin,desc);
  LogFile << "Model description: " << desc << endl;
  cout << endl << "Please make shure that if you have a laptop your touchpad is enabled" << endl;

  #ifdef _WIN32
  system("pause");
  #endif

  cout << endl << "Disabling all LEDs..." << endl;
  
  // Disable all the LEDs of the AlienFX device
  AlienfxWaitForBusy();
  AlienfxReset(ALIENFX_ALL_OFF);
  Sleep(2);
  AlienfxWaitForReady();
  AlienfxSetColor(ALIENFX_STAY,1,0xFFFFFF,0);
  AlienfxEndLoopBlock();
  AlienfxEndTransmitionAndExecute();
  Sleep(100);

  input = 0;
  while(input != 'y' && input != 'n'){
    cout << endl << "Are all LEDs off? (y/n)" << endl;
    cin.get(input);
    cin.ignore(100,'\n');
  }
  if(input == 'n'){
    cout << endl << "Please name the LEDs which are still on" << endl;
    getline(cin,desc);
    LogFile << "LEDs still on: " << desc << endl;
  }
  cout << endl << "Now all possible LEDs will be tested" << endl;

  int leds = 0x000001;
  
  int ids[] = {
    0x0001,
    0x0002,
    0x0008,
    0x0004,
    0x0010,
    0x0020,
    0x0040,
    0x0080,
    0x0100,
    0x0200,
    0x1c00,
    0x2000,
    0x4000,
    0x6000,
    0x6000,
    0x010000,
    0x0f9fff};

  for(int i = 0; i < 17; i++){
    leds = ids[i];
    cout << endl << "Testing LED " << i << endl;
    
    // make the current LED blink
    AlienfxWaitForBusy();
    AlienfxReset(ALIENFX_ALL_ON);
    Sleep(2);
    AlienfxWaitForReady();
    AlienfxSetColor(ALIENFX_BLINK,1,leds,ON_COLOR);
    AlienfxEndLoopBlock();
    AlienfxEndTransmitionAndExecute();
    Sleep(100);

    input = 0;

    while(input != 'y' && input != 'n'){
      cout << endl << "Did something change (y/n)" << endl;
      cin.get(input);
      cin.ignore(100,'\n');
    }

    if(input == 'y'){
      cout << endl << "What did change?" << endl;
      getline(cin,desc);
      LogFile << "Led 0x" << hex << leds << dec << ": " << desc << endl;
    }

    // turn the current LED off
    AlienfxWaitForBusy();
    AlienfxReset(ALIENFX_ALL_ON);
    Sleep(2);
    AlienfxWaitForReady();
    AlienfxSetColor(ALIENFX_BLINK,1,leds,OFF_COLOR);
    AlienfxEndLoopBlock();
    AlienfxEndTransmitionAndExecute();
    Sleep(100);
    
    //leds = leds << 1;
  }
  LogFile.close();
  cout << "Testing complete. Please send the 'results.log' file to webmaster@benjamin-thaut.de" << endl;

  #ifdef _WIN32
  system("pause");
  #endif

  return 0;
}
