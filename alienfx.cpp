#include "alienfx.h"
#include <Setupapi.h>
#include <ddk/hidsdi.h>
#include <ddk/hidclass.h>
#include <boost/scoped_ptr.hpp>
#include <string>

HANDLE hDeviceHandle = NULL;
std::wstring AlienfxDeviceName;
bool AlienfxNew = false;

bool FindDevice(int pVendorId, int pProductId, std::wstring& pDevicePath){
  GUID guid;
  bool flag = false;
  pDevicePath = L"";
  HidD_GetHidGuid(&guid);
  HDEVINFO hDevInfo = SetupDiGetClassDevsA(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
  if (hDevInfo == INVALID_HANDLE_VALUE)
  {
      //std::cout << "Couldn't get guid";
      return false;
  }
  unsigned int dw = 0;
  SP_DEVICE_INTERFACE_DATA deviceInterfaceData;

  unsigned int lastError = 0;
  while (!flag)
  {
      deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
      if (!SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &guid, dw, &deviceInterfaceData))
      {
          lastError = GetLastError();
          return false;
      }
      dw++;
      DWORD dwRequiredSize = 0;
      if(SetupDiGetDeviceInterfaceDetailW(hDevInfo,&deviceInterfaceData,NULL,0,&dwRequiredSize,NULL)){
        //std::cout << "Getting the needed buffer size failed";
        return false;
      }
      //std::cout << "Required size is " << dwRequiredSize << std::endl;
      if(GetLastError() != ERROR_INSUFFICIENT_BUFFER){
        //std::cout << "Last error is not ERROR_INSUFFICIENT_BUFFER";
        return false;
      }
      boost::scoped_ptr<SP_DEVICE_INTERFACE_DETAIL_DATA> deviceInterfaceDetailData((SP_DEVICE_INTERFACE_DETAIL_DATA*)new char[dwRequiredSize]);
      deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
      if (SetupDiGetDeviceInterfaceDetailW(hDevInfo, &deviceInterfaceData, deviceInterfaceDetailData.get(), dwRequiredSize, NULL, NULL))
      {
          std::wstring devicePath = deviceInterfaceDetailData->DevicePath;
          HANDLE hDevice = CreateFile(devicePath.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
          if (hDevice != INVALID_HANDLE_VALUE)
          {
              boost::scoped_ptr<HIDD_ATTRIBUTES> attributes(new HIDD_ATTRIBUTES);
              attributes->Size = sizeof(HIDD_ATTRIBUTES);
              if (HidD_GetAttributes(hDevice, attributes.get())){
                if(((attributes->VendorID == pVendorId) && (attributes->ProductID == pProductId)))
                {
                  flag = true;
                  pDevicePath = devicePath;
                }
                else {
                  //std::cout << "ProductID and VendorID does not match" << std::endl;
                }
              }
              else {
                //std::cout << "Failed to get attributes" << std::endl;
              }
              CloseHandle(hDevice);
          }
          else
          {
              lastError = GetLastError();
              //std::cout << "Failed to open file" << std::endl;
          }
      }
  }
  return flag;
}

bool AlienfxInit(){
  #ifdef ALIENFX_DEBUG
  std::cout << "FindDevice" << std::endl;
  #endif
  if(FindDevice(0x187c,0x511,AlienfxDeviceName)){
    hDeviceHandle = CreateFile(AlienfxDeviceName.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if(hDeviceHandle == NULL)
      return false;
    return true;
  }
  if(FindDevice(0x187c,0x512,AlienfxDeviceName)){
    #ifdef ALIENFX_DEBUG
    std::cout << "Opening new alienfx device" << std::endl;
    #endif
    hDeviceHandle = CreateFile(AlienfxDeviceName.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if(hDeviceHandle == NULL)
      return false;
    AlienfxNew = true;
    return true;
  }
  if(FindDevice(0x187c,0x514,AlienfxDeviceName)){
    #ifdef ALIENFX_DEBUG
    std::cout << "Opening m11x alienfx device" << std::endl;
    #endif
    hDeviceHandle = CreateFile(AlienfxDeviceName.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if(hDeviceHandle == NULL)
      return false;
    AlienfxNew = true;
    return true;
  }
  if(FindDevice(0x187c,0x520,AlienfxDeviceName)){
    #ifdef ALIENFX_DEBUG
    std::cout << "Opening M17x R3 alienfx device" << std::endl;
    #endif
    hDeviceHandle = CreateFile(AlienfxDeviceName.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if(hDeviceHandle == NULL)
      return false;
    AlienfxNew = true;
    return true;
  }
  return false;
}

bool AlienfxReinit(){
  CloseHandle(hDeviceHandle);
  hDeviceHandle = CreateFile(AlienfxDeviceName.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
  if(hDeviceHandle == NULL)
    return false;
  return true;
}

void AlienfxDeinit(){
  if(hDeviceHandle != NULL){
    CloseHandle(hDeviceHandle);
    hDeviceHandle = NULL;
  }
}

bool WriteDevice(byte *pBuffer, size_t pBytesToWrite, size_t& pBytesWritten){
  if(AlienfxNew){
    pBuffer[0] = 0x02;
    return DeviceIoControl(hDeviceHandle,IOCTL_HID_SET_OUTPUT_REPORT,pBuffer,pBytesToWrite,NULL,0,(DWORD*)&pBytesWritten,NULL);
  }
  return WriteFile(hDeviceHandle, pBuffer, pBytesToWrite, (DWORD*)&pBytesWritten, NULL);
}

bool ReadDevice(byte *pBuffer, size_t pBytesToRead, size_t& pBytesRead){
  if(AlienfxNew){
    return DeviceIoControl(hDeviceHandle,IOCTL_HID_GET_INPUT_REPORT,NULL,0,pBuffer,pBytesToRead,(DWORD*)&pBytesRead,NULL);
  }
  return ReadFile(hDeviceHandle,pBuffer,pBytesToRead,(DWORD*)&pBytesRead,NULL);
}

void AlienfxReset(byte pOptions){
  size_t BytesWritten;
  byte Buffer[] = { 0x00 ,0x07 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 };
  Buffer[2] = pOptions;
  WriteDevice(Buffer, 9, BytesWritten);
}

void AlienfxSetColor(byte pAction, byte pSetId, int pLeds, int pColor){
  size_t BytesWritten;
  byte Buffer[] = { 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 };
  Buffer[1] = pAction;
  Buffer[2] = pSetId;
  Buffer[3] = (pLeds & 0xFF0000) >> 16;
  Buffer[4] = (pLeds & 0x00FF00) >> 8;
  Buffer[5] = (pLeds & 0x0000FF);
  Buffer[6] = (pColor & 0xFF0000) >> 16;
  Buffer[7] = (pColor & 0x00FF00) >> 8;
  Buffer[8] = (pColor & 0x0000FF);
  WriteDevice(Buffer, 9, BytesWritten);
}

void AlienfxEndLoopBlock(){
  size_t BytesWritten;
  byte Buffer[] = { 0x00 ,0x04 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 };
  WriteDevice(Buffer, 9, BytesWritten);
}

void AlienfxEndTransmitionAndExecute(){
  size_t BytesWritten;
  byte Buffer[] = { 0x00 ,0x05 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 };
  WriteDevice(Buffer, 9, BytesWritten);
}

void AlienfxStoreNextInstruction(byte pStorageId){
  size_t BytesWritten;
  byte Buffer[] = { 0x00 ,0x08 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 };
  Buffer[2] = pStorageId;
  WriteDevice(Buffer, 9, BytesWritten);
}

void AlienfxEndStorageBlock(){
  size_t BytesWritten;
  byte Buffer[] = { 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 };
  WriteDevice(Buffer, 9, BytesWritten);
}

void AlienfxSaveStorageData(){
  size_t BytesWritten;
  byte Buffer[] = { 0x00 ,0x09 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 };
  WriteDevice(Buffer, 9, BytesWritten);
}

void AlienfxSetSpeed(int pSpeed){
  size_t BytesWritten;
  byte Buffer[] = { 0x00 ,0x0E ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 };
  Buffer[2] = (pSpeed & 0xFF00) >> 8;
  Buffer[3] = (pSpeed & 0x00FF);
  WriteDevice(Buffer, 9, BytesWritten);
}

byte AlienfxGetDeviceStatus(){
  #ifdef ALIENFX_DEBUG
  std::cout << "AlienfxGetDeviceStatus";
  #endif
  size_t BytesWritten;
  byte Buffer[] = { 0x00 ,0x06 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 };
  WriteDevice(Buffer, 9, BytesWritten);
  if(AlienfxNew)
    Buffer[0] = 0x01;
  ReadDevice(Buffer, 9, BytesWritten);
  #ifdef ALIENFX_DEBUG
  for(int i=0;i<9;i++){
    std::cout << " " << std::hex << (int)Buffer[i];
  }
  std::cout << std::endl;
  #endif
  if(AlienfxNew){
    if (Buffer[0] == 0x01)
      return 0x06;
    return Buffer[0];
  }
  else
    return Buffer[1];
}

byte AlienfxWaitForReady(){
  byte status = AlienfxGetDeviceStatus();
  for(int i=0;i<5 && status != ALIENFX_READY;i++){
    if(status == ALIENFX_DEVICE_RESET)
      return status;
    Sleep(1);
    status = AlienfxGetDeviceStatus();
  }
  return status;
}

byte AlienfxWaitForBusy(){
  byte status = AlienfxGetDeviceStatus();
  for(int i=0;i<5 && status != ALIENFX_BUSY;i++){
    if(status == ALIENFX_DEVICE_RESET)
      return status;
    Sleep(1);
    status = AlienfxGetDeviceStatus();
  }
  return status;
}
