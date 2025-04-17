#include "AudioMixerControl.h"
#include <stdio.h>

// Start processing
HRESULT AudioMixerControl::StartProcessing() {
    if (m_isProcessing) {
        return S_FALSE; // Already processing
    }
    m_isProcessing = true;
    printf("Processing started.\n");
    return S_OK;
}

// Stop processing
HRESULT AudioMixerControl::StopProcessing() {
    if (!m_isProcessing) {
        return S_FALSE; // Already stopped
    }
    m_isProcessing = false;
    printf("Processing stopped.\n");
    return S_OK;
}

// QueryInterface implementation
STDMETHODIMP AudioMixerControl::QueryInterface(REFIID riid, void** ppv) {
    if (riid == __uuidof(IAudioMixerControl)) {
        *ppv = static_cast<IAudioMixerControl*>(this);
    }
    else if (riid == IID_IUnknown) {
        *ppv = static_cast<IUnknown*>(this);
    }
    else {
        *ppv = nullptr;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

// AddRef implementation
ULONG AudioMixerControl::AddRef() {
    return InterlockedIncrement(&m_cRef);
}

// Release implementation
ULONG AudioMixerControl::Release() {
    ULONG ref = InterlockedDecrement(&m_cRef);
    if (ref == 0) {
        delete this;
    }
    return ref;
}
