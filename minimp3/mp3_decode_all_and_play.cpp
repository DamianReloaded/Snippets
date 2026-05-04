#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <portaudio.h>
#define MINIMP3_IMPLEMENTATION
#include <minimp3.h>
#include <minimp3_ex.h>

struct AudioData
{
    std::vector<float> samples;
    size_t position = 0;
    int channels = 0;
};

static int audioCallback(const void*, void* outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo*,
                         PaStreamCallbackFlags,
                         void* userData)
{
    auto* data = static_cast<AudioData*>(userData);
    float* out = static_cast<float*>(outputBuffer);

    size_t framesLeft = data->samples.size() / data->channels - data->position;
    size_t framesToCopy = std::min<size_t>(framesPerBuffer, framesLeft);

    if (framesToCopy > 0)
    {
        std::memcpy(out,
                    &data->samples[data->position * data->channels],
                    framesToCopy * data->channels * sizeof(float));
        data->position += framesToCopy;
    }
    else
    {
        std::fill(out, out + framesPerBuffer * data->channels, 0.0f);
        return paComplete;
    }

    return paContinue;
}

bool loadMP3(const char* filename, AudioData& data, int& sampleRate)
{
    mp3dec_ex_t mp3;
    if (mp3dec_ex_open(&mp3, filename, MP3D_SEEK_TO_SAMPLE) != 0)
    {
        std::cerr << "Failed to open MP3 file.\n";
        return false;
    }

    data.channels = mp3.info.channels;
    sampleRate = mp3.info.hz;

    std::vector<mp3d_sample_t> pcm(mp3.samples);
    size_t samplesRead = mp3dec_ex_read(&mp3, pcm.data(), mp3.samples);
    pcm.resize(samplesRead);

    // Convert to float
    data.samples.resize(pcm.size());
    for (size_t i = 0; i < pcm.size(); ++i)
        data.samples[i] = pcm[i] / 32768.0f;

    mp3dec_ex_close(&mp3);
    return true;
}

int main(int argc, char* argv[])
{
    /*
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <file.mp3>\n";
        return 1;
    }
    */

    std::string filename = "data/music/music01.mp3";

    AudioData audioData;
    int sampleRate = 0;

    if (!loadMP3(filename.c_str(), audioData, sampleRate)) //argv[1]
        return 1;

    if (Pa_Initialize() != paNoError)
    {
        std::cerr << "PortAudio init failed.\n";
        return 1;
    }

    PaStreamParameters outputParams;
    outputParams.device = Pa_GetDefaultOutputDevice();
    outputParams.channelCount = audioData.channels;
    outputParams.sampleFormat = paFloat32;
    outputParams.suggestedLatency =
        Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;

    PaStream* stream;
    if (Pa_OpenStream(&stream, nullptr, &outputParams,
                      sampleRate, 256, paNoFlag,
                      audioCallback, &audioData) != paNoError)
    {
        std::cerr << "Failed to open stream.\n";
        Pa_Terminate();
        return 1;
    }

    Pa_StartStream(stream);
    while (Pa_IsStreamActive(stream) == 1)
        Pa_Sleep(100);

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}