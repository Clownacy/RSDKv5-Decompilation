
uint8 AudioDevice::contextInitialized;

SDL_AudioDeviceID AudioDevice::device;
SDL_AudioSpec AudioDevice::deviceSpec;

bool32 AudioDevice::Init()
{
    SDL_InitSubSystem(SDL_INIT_AUDIO);
    if (!contextInitialized) {
        contextInitialized = true;
        InitAudioChannels();
    }

    SDL_AudioSpec want;
    want.freq     = AUDIO_FREQUENCY;
    want.format   = AUDIO_S16SYS;
    want.samples  = MIX_BUFFER_SIZE / AUDIO_CHANNELS;
    want.channels = AUDIO_CHANNELS;
    want.callback = AudioCallback;

    audioState = false;
    if ((device = SDL_OpenAudioDevice(nullptr, 0, &want, &deviceSpec, 0)) > 0) {
        SDL_PauseAudioDevice(device, SDL_FALSE);
        audioState = true;
    }
    else {
        PrintLog(PRINT_NORMAL, "ERROR: Unable to open audio device!");
        PrintLog(PRINT_NORMAL, "ERROR: %s", SDL_GetError());
    }

    return true;
}

void AudioDevice::Release()
{
    LockAudioDevice();

    UnloadStream();

    UnlockAudioDevice();

    SDL_CloseAudioDevice(AudioDevice::device);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void AudioDevice::InitAudioChannels()
{
    for (int32 i = 0; i < CHANNEL_COUNT; ++i) {
        channels[i].soundID = -1;
        channels[i].state   = CHANNEL_IDLE;
    }

    GEN_HASH_MD5("Stream Channel 0", sfxList[SFX_COUNT - 1].hash);
    sfxList[SFX_COUNT - 1].scope              = SCOPE_GLOBAL;
    sfxList[SFX_COUNT - 1].maxConcurrentPlays = 1;
    sfxList[SFX_COUNT - 1].length             = MIX_BUFFER_SIZE;
    AllocateStorage((void **)&sfxList[SFX_COUNT - 1].buffer, MIX_BUFFER_SIZE * sizeof(int16), DATASET_MUS, false);

    initializedAudioChannels = true;
}

void AudioDevice::AudioCallback(void *data, uint8 *stream, int32 len)
{
    (void)data; // Unused

    AudioDevice::ProcessAudioMixing(stream, len / sizeof(int16));
}
