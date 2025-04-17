#include <windows.h>  
#include <comdef.h>  
#include "../AudioMixerApo/AudioMixerControl.h"  

void PrintHRESULT(HRESULT hr, const char* operation) {  
   if (FAILED(hr)) {  
       _com_error err(hr);  
       printf("%s failed: 0x%08X - %ls\n", operation, hr, err.ErrorMessage());  
   }  
}  

int main() {  
   HRESULT hr;  

   // Initialize COM  
   hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);  
   if (FAILED(hr)) {  
       PrintHRESULT(hr, "CoInitializeEx");  
       return 1;  
   }  

   // Create the control instance via COM
   IAudioMixerControl* pControl = nullptr;
   hr = CoCreateInstance(__uuidof(IAudioMixerControl), NULL, CLSCTX_INPROC_SERVER, __uuidof(IAudioMixerControl), (void**)&pControl);
   if (FAILED(hr)) {
       PrintHRESULT(hr, "CoCreateInstance");
       CoUninitialize();
       return 1;
   }

   // Send commands  
   printf("Starting processing...\n");  
   hr = pControl->StartProcessing();  
   PrintHRESULT(hr, "StartProcessing");  

   printf("Press Enter to stop processing...\n");  
   (void)getchar();  

   printf("Stopping processing...\n");  
   hr = pControl->StopProcessing();  
   PrintHRESULT(hr, "StopProcessing");  

   // Clean up  
   pControl->Release();  
   CoUninitialize();  
   return 0;  
}
