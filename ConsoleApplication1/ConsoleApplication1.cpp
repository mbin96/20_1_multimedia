#include <Windows.h> 
#include <mmsystem.h>
#include <stdio.h>
#include <conio.h> // for getch
#include <iostream>
#pragma comment(lib, "winmm.lib")
#pragma warning(disable : 4996)

#define Fs 16000 
#define Bits 16 
#define REC_DUR 6
typedef struct {
    INT16 buffer[Fs];
    INT16* next;
}buff;

void MovingAverageFilter(short* in, short* out, int N, int tap) {
    int i, j, M;
    float t;
    for (i = 0; i < N; i++) { 
        for (j = 0, t = 0, M = 0; j < tap; j++) { 
            if (i - j >= 0) {
                t += in[i - j]; M++; 
            } 
        } 
        out[i] = (short)(t / M); 
    }
}

void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
    if (uMsg == WIM_DATA)
        printf("데이타가 다 채워졌읍니다.\n");
}
void CALLBACK waveOutProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
    if (uMsg == WIM_DATA)
        printf("데이타가 다 채워졌읍니다.\n");
}
int main()
{
    WAVEFORMATEX my_wave_format;
    HWAVEIN h_input;
    HWAVEOUT h_output;
    WAVEHDR w_header;

    short int* Buf = new short int[Fs * REC_DUR];
    
    my_wave_format.wFormatTag = WAVE_FORMAT_PCM;
    my_wave_format.nChannels = 1;
    my_wave_format.nSamplesPerSec = Fs;
    my_wave_format.wBitsPerSample = Bits;
    my_wave_format.nBlockAlign = my_wave_format.nChannels * my_wave_format.wBitsPerSample / 8;
    my_wave_format.nAvgBytesPerSec = my_wave_format.nSamplesPerSec * my_wave_format.nBlockAlign;
    my_wave_format.cbSize = 0;

    waveInOpen(&h_input, WAVE_MAPPER, &my_wave_format, (DWORD_PTR)waveInProc, 0L, CALLBACK_FUNCTION);
    waveOutOpen(&h_output, WAVE_MAPPER, &my_wave_format, (DWORD_PTR)waveOutProc, 0L, CALLBACK_FUNCTION);

    w_header.lpData = (LPSTR)Buf;
    w_header.dwBufferLength = Fs * REC_DUR * sizeof(short);
    w_header.dwFlags = w_header.dwLoops = 0L;
    
    waveInPrepareHeader(h_input, &w_header, sizeof(WAVEHDR));
    memset(Buf, 0, Fs * REC_DUR * sizeof(short));
    waveInAddBuffer(h_input, &w_header, sizeof(WAVEHDR));
    
    printf("아무키나 누르면 녹음이 시작됩니다.\n");
    _getch();
   
    waveInStart(h_input);
    Sleep(REC_DUR*1000+100);

    FILE* fp = fopen("6sec.snd", "wb"); 
    fwrite(Buf, sizeof(short), Fs * REC_DUR, fp); 
    fclose(fp); 
    //waveInReset(h_input); 
    //waveInUnprepareHeader(h_input, &w_header, sizeof(WAVEHDR));
    //waveInClose(h_input);

    printf("아무키나 누르면 재생이 시작됩니다.\n");
    _getch();
    
    waveOutPrepareHeader(h_output, &w_header, sizeof(WAVEHDR));
    waveOutWrite(h_output, &w_header, sizeof(WAVEHDR));
    Sleep(4000);
    
    waveInReset(h_input);
    waveInUnprepareHeader(h_input, &w_header, sizeof(WAVEHDR));
    waveInClose(h_input);
    waveOutReset(h_output);
    waveOutClose(h_output);
    return 0;
}