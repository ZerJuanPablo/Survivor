#pragma once
#include <unordered_map>
#include <string>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_init.h>
#include <fmt/core.h>

// Estructura para guardar la data del WAV
struct AudioData {
    SDL_AudioSpec spec;
    Uint8* buffer = nullptr;
    Uint32 buffer_size = 0;
};

class AudioManager {
public:
    // Constructor/Destructor
    AudioManager()  { 
        // Inicializa el subsistema de audio sólo una vez
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
            fmt::print("Error al inicializar audio: {}\n", SDL_GetError());
        }
    }

    ~AudioManager() {
        // Libera cada uno de los buffers cargados
        for (auto& [key, audioData] : _sounds) {
            if (audioData.buffer) {
                SDL_free(audioData.buffer);
            }
        }
        _sounds.clear();

        // Destruye el stream si está abierto
        if (_audio_stream) {
            SDL_DestroyAudioStream(_audio_stream);
            _audio_stream = nullptr;
        }
    }

    // Inicializar (o re-inicializar) el dispositivo de salida
    bool initDevice() {
        _audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                                  nullptr, nullptr, nullptr);
        if (!_audio_stream) {
            fmt::print("Error abriendo el dispositivo de audio: {}\n", SDL_GetError());
            return false;
        }
        // Obtenemos el formato del dispositivo para saber a qué formato convertir
        SDL_GetAudioDeviceFormat(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &_device_format, nullptr);
        return true;
    }

    // Carga un .wav en memoria y lo asocia con un nombre (por ejemplo "shot", "hit", etc.)
    bool loadSound(const std::string& soundName, const std::string& wavPath) {
        AudioData audioData;
        if (!SDL_LoadWAV(wavPath.c_str(), &audioData.spec, &audioData.buffer, &audioData.buffer_size)) {
            fmt::print("Error cargando WAV '{}': {}\n", wavPath, SDL_GetError());
            return false;
        }
        _sounds[soundName] = audioData;
        return true;
    }

    // Reproduce el sonido que hayas cargado con "soundName"
    void playSound(const std::string& soundName) {
        // Si no hemos abierto el stream o no existe el sonido, no hacemos nada.
        if (!_audio_stream) {
            fmt::print("Stream de audio nulo. Llama initDevice() antes de reproducir.\n");
            return;
        }
        auto it = _sounds.find(soundName);
        if (it == _sounds.end()) {
            fmt::print("El sonido '{}' no está cargado.\n", soundName);
            return;
        }
        // Ajustamos la conversión de formato sólo si es necesario (algunos proyectos lo hacen una vez).
        // Si vas a reproducir varios sonidos con sample rate diferentes, sí haz el SetAudioStreamFormat
        // en cada play. De lo contrario, puedes ponerlo en loadSound si vas a normalizarlos todos igual.
        if (!SDL_SetAudioStreamFormat(_audio_stream, &it->second.spec, &_device_format)) {
            fmt::print("Error en SetAudioStreamFormat: {}\n", SDL_GetError());
        }

        // Mandamos los datos al stream
        if (!SDL_PutAudioStreamData(_audio_stream, it->second.buffer, it->second.buffer_size)) {
            fmt::print("Error en PutAudioStreamData: {}\n", SDL_GetError());
        }

        // Iniciamos la reproducción
        if (!SDL_ResumeAudioStreamDevice(_audio_stream)) {
            fmt::print("Error en ResumeAudioStreamDevice: {}\n", SDL_GetError());
        }
    }

private:
    SDL_AudioSpec _device_format {};            // Formato de reproducción del dispositivo
    SDL_AudioStream* _audio_stream = nullptr;   // Stream para reproducir
    std::unordered_map<std::string, AudioData> _sounds; // Map de sonidos cargados
};
