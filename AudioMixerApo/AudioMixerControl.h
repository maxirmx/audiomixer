#pragma once
#include <Unknwn.h>

// Define a custom interface for controlling the APO
struct __declspec(uuid("B1234567-89AB-4CDE-0123-456789ABCDEF")) IAudioMixerControl : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE StartProcessing() = 0;
    virtual HRESULT STDMETHODCALLTYPE StopProcessing() = 0;
};

// Define a class that implements the IAudioMixerControl interface
class AudioMixerControl : public IAudioMixerControl {
private:
    LONG m_cRef; // Reference count for COM
    bool m_isProcessing; // Track whether processing is active

public:
    AudioMixerControl() : m_cRef(1), m_isProcessing(false) {
    }

    ~AudioMixerControl() {
    }

    // IAudioMixerControl methods
    HRESULT STDMETHODCALLTYPE StartProcessing() override;
    HRESULT STDMETHODCALLTYPE StopProcessing() override;

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;
};