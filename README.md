# audiomixer
Audio mixer


Soundpad-style APO Voice Mixer

Build Instructions:
1. Open the solution in Visual Studio (x64)
2. Build MicrophoneAPO (DLL)
3. Build APOController (CLI)

To Install:
- Enable Test Signing Mode:
  bcdedit /set testsigning on
  shutdown /r /t 0

- Use `devcon install AudioEffect.inf` to attach the APO to your microphone

To Run:
- Use APOController.exe path\to\audio.wav
- Audio will be injected into mic stream for apps like Zoom

Limitations:
- WAV file must be 16-bit PCM, 48kHz stereo
- Mixes only basic float samples; real implementation should convert properly
