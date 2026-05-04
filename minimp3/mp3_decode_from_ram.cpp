#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <portaudio.h>

#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
#include "minimp3_ex.h"

struct AudioStream
{
    mp3dec_ex_t decoder{};
    std::vector<unsigned char> compressed;
    bool finished = false;
};

// ---- Callback ----
static int audioCallback(const void*, void* outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo*,
                         PaStreamCallbackFlags,
                         void* userData)
{
    auto* stream = static_cast<AudioStream*>(userData);
    float* out = static_cast<float*>(outputBuffer);

    const int channels = stream->decoder.info.channels;
    const size_t totalSamples = framesPerBuffer * channels;

    // temporary decode buffer for int16 samples
    std::vector<mp3d_sample_t> temp(totalSamples);

    size_t decoded = mp3dec_ex_read(&stream->decoder, temp.data(), totalSamples);

    if (decoded == 0)
    {
        std::fill(out, out + totalSamples, 0.0f);
        stream->finished = true;
        return paComplete;
    }

    // Convert int16 → float
    for (size_t i = 0; i < decoded; ++i)
        out[i] = temp[i] / 32768.0f;

    // Pad the remainder with silence if short
    if (decoded < totalSamples)
        std::fill(out + decoded, out + totalSamples, 0.0f);

    return paContinue;
}

// ---- Utility: read entire file ----
std::vector<unsigned char> readFileToMemory(const char* filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file)
        return {};
    return {std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()};
}

// ---- Main ----
int main()
{
    const char* filename = "data/music/music01.mp3";

    AudioStream stream;
    stream.compressed = readFileToMemory(filename);
    if (stream.compressed.empty())
    {
        std::cerr << "Failed to read MP3 file.\n";
        return 1;
    }

    if (mp3dec_ex_open_buf(&stream.decoder,
                           stream.compressed.data(),
                           stream.compressed.size(),
                           MP3D_SEEK_TO_SAMPLE) != 0)
    {
        std::cerr << "Failed to open MP3 from memory.\n";
        return 1;
    }

    const int channels = stream.decoder.info.channels;
    const int sampleRate = stream.decoder.info.hz;

    if (Pa_Initialize() != paNoError)
    {
        std::cerr << "PortAudio init failed.\n";
        return 1;
    }

    PaStreamParameters outputParams{};
    outputParams.device = Pa_GetDefaultOutputDevice();
    outputParams.channelCount = channels;
    outputParams.sampleFormat = paFloat32;
    outputParams.suggestedLatency =
        Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;

    PaStream* paStream;
    if (Pa_OpenStream(&paStream,
                      nullptr,
                      &outputParams,
                      sampleRate,
                      1024, // frames per buffer
                      paNoFlag,
                      audioCallback,
                      &stream) != paNoError)
    {
        std::cerr << "Failed to open PortAudio stream.\n";
        Pa_Terminate();
        return 1;
    }

    Pa_StartStream(paStream);

    while (!stream.finished && Pa_IsStreamActive(paStream))
        Pa_Sleep(50);

    Pa_StopStream(paStream);
    Pa_CloseStream(paStream);
    Pa_Terminate();
    mp3dec_ex_close(&stream.decoder);

    return 0;
}
