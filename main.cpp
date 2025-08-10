#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const int SR = 44100;           // Частота семплирования
const float EAR_D = 0.204f;     // Расстояние между ушами
const float SND_SPD = 340.29f;  // скорость звука

struct WavHeader {
    char chunkId[4] = { 'R', 'I', 'F', 'F' };
    uint32_t chunkSize;
    char format[4] = { 'W', 'A', 'V', 'E' };
    char subchunk1Id[4] = { 'f', 'm', 't', ' ' };
    uint32_t subchunk1Size = 16;
    uint16_t audioFormat = 1; // PCM
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample = 32;
    char subchunk2Id[4] = { 'd', 'a', 't', 'a' };
    uint32_t subchunk2Size;
};

class Sound3D {
public:
    Sound3D(float dist = 1.0f, float rot = 1.0f) : d(dist), v(rot) {}

    void gen(int ms) {
        int N = ms * SR / 1000;
        buf.resize(N * 2);

        float da = 360.0f * v / (ms / 1000.0f) / SR;
        float a = 0.0f;

        for (int i = 0; i < N; i++) {
            float dt = calcDT(a);
            int ds = static_cast<int>(fabs(dt) * SR);
            float vol = 1.0f / (1.0f + d);

            float s = 0.5f * sin(2.0f * M_PI * 440.00f * i / SR) * vol;

            if (dt > 0) {
                if (i + ds < N) buf[(i + ds) * 2] += s;
                buf[i * 2 + 1] += s;
            }
            else {
                if (i + ds < N) buf[(i + ds) * 2 + 1] += s;
                buf[i * 2] += s;
            }

            a += da;
            if (a >= 360.0f) a -= 360.0f;
        }

        normalize();
    }

    float calcDT(float deg) {
        float rad = deg * M_PI / 180.0f;
        float dt_max = EAR_D / SND_SPD;

        if (fabs(rad) <= M_PI / 2) {
            return dt_max * (sin(rad) + rad);
        }
        else {
            float abs_rad = fabs(rad);
            return copysignf(1.0f, sin(rad)) * dt_max * (M_PI - abs_rad + sin(abs_rad));
        }
    }

    void normalize() {
        float max = 0.0f;
        for (float& s : buf) if (fabs(s) > max) max = fabs(s);
        if (max > 0.0f) for (float& s : buf) s /= max;
    }

    void saveToWav(const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if (!file) {
            std::cerr << "Error: Cannot open file for writing" << std::endl;
            return;
        }

        WavHeader header;
        header.numChannels = 2;
        header.sampleRate = SR;
        header.bitsPerSample = 32;
        header.byteRate = SR * header.numChannels * header.bitsPerSample / 8;
        header.blockAlign = header.numChannels * header.bitsPerSample / 8;
        header.subchunk2Size = buf.size() * sizeof(float);
        header.chunkSize = 36 + header.subchunk2Size;

        file.write(reinterpret_cast<char*>(&header), sizeof(header));
        file.write(reinterpret_cast<char*>(buf.data()), buf.size() * sizeof(float));

        std::cout << "File " << filename << " saved successfully" << std::endl;
    }

private:
    std::vector<float> buf;
    float d;  // Дистанция
    float v;  // Скорость вращения
};

int main() {
    float distance, speed;
    int duration;
    std::string filename = "output.wav";

    std::cout << "Sound Generator" << std::endl;
    std::cout << "------------------" << std::endl;

    // Ввод дистанции
    while (true) {
        std::cout << "Enter sound source distance (0.1-10 meters): ";
        std::cin >> distance;
        if (distance >= 0.1f && distance <= 10.0f) break;
        std::cout << "Invalid input. Please enter a value between 0.1 and 10." << std::endl;
    }

    // Ввод скорости вращения
    while (true) {
        std::cout << "Enter rotation speed (0.1-5 rotations/sec): ";
        std::cin >> speed;
        if (speed >= 0.1f && speed <= 5.0f) break;
        std::cout << "Invalid input. Please enter a value between 0.1 and 5." << std::endl;
    }

    // Вввод длительности
    while (true) {
        std::cout << "Enter sound duration (100-10000 milliseconds): ";
        std::cin >> duration;
        if (duration >= 100 && duration <= 10000) break;
        std::cout << "Invalid input. Please enter a value between 100 and 10000." << std::endl;
    }

    std::cout << "\nGenerating sound with parameters:" << std::endl;
    std::cout << "Distance: " << distance << " meters" << std::endl;
    std::cout << "Rotation speed: " << speed << " rotations/sec" << std::endl;
    std::cout << "Duration: " << duration << " ms" << std::endl;

    Sound3D sound(distance, speed);
    sound.gen(duration);
    sound.saveToWav(filename);

    std::cout << "\nDone! Press Enter to exit...";
    std::cin.ignore();
    std::cin.get();

    return 0;
}