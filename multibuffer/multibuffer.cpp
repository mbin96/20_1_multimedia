



#include <Windows.h> 
#pragma comment(lib, "winmm.lib") 
#include <iostream>
#include <conio.h>
#pragma warning(disable : 4996)
using namespace std;
HANDLE hEvent_BufferReady;
HANDLE hEvent_FinishedPlaying;
#define Samplerate 16000 
#define LEN_BUF 16000 
#define nSec 5 
int filterTap = 0;
int iBuf;
int iplaying;
int center = 0;
//int ifilter;
unsigned long result;
short spt[16000];
//끊어지지 않는 버퍼를 위해 in 버터를 조금더 크게
short in[LEN_BUF + 30], out[LEN_BUF];
char filter_on = 0;
HWAVEIN hWaveIn;
HWAVEOUT hWaveOut;
WAVEFORMATEX pFormat;


//하나의 음성데이터에 각각 다른 필터를 적용하여 저장하기
FILE* fp = fopen("result_.snd", "wb");//출력되는 음성
FILE* fpraw = fopen("result_raw_.raw", "wb");//필터안된음성=
FILE* fp3 = fopen("result_tap3_.raw", "wb");//tpa3 필터음성
FILE* fp5 = fopen("result_tap5_.raw", "wb");//tap5 필터음선
FILE* fp9 = fopen("result_tap9_.raw", "wb");// tap9 필터음성

enum
{
    NUM_BUF = 8
};
WAVEHDR header[NUM_BUF];

void MovingAverageFilter(short* in, short* out, int N, int tap) {
    int i, j, M;
    float t;
    //N+tap으로 변경
    //이전 버퍼의 데이터를 앞에 넣어줬기때문에 끊어지지 않는 녹음 가능
    for (i = 0; i < N + tap; i++)
    {
        for (j = 0, t = 0, M = 0; j < tap; j++)
        {
            if (i - j >= 0) {
                //가중치 여부
                //가중치 변수 center가 1인경우 가운데 값을 2배로 더하여줌
                if (center == 1) {
                    if (j == (int)(tap / 2)) {
                        t += 2 * in[i - j];
                        M++;
                    }
                    else {
                        t += in[i - j];
                    }
                    M++;
                }
                else {
                    t += in[i - j];
                    M++;
                }

            }
        }
        if (tap <= i)
            out[i - tap] = (short)(t / M);
    }
}


DWORD WINAPI RecordingWaitingThread(LPVOID ivalue)
{
    while (1)
    {
        WaitForSingleObject(hEvent_BufferReady, INFINITE);
        result = waveInUnprepareHeader(hWaveIn, &header[iBuf], sizeof(WAVEHDR));
        iplaying = iBuf;
        //ifilter = (iplaying - 1)% NUM_BUF;
        ++iBuf;
        if (iBuf == NUM_BUF)
            iBuf = 0;
        result = waveInPrepareHeader(hWaveIn, &header[iBuf], sizeof(WAVEHDR));
        result = waveInAddBuffer(hWaveIn, &header[iBuf], sizeof(WAVEHDR));
        // memcpy(spt, header[iBuf].lpData, LEN_BUF * sizeof(short));
        // memcpy(header[iBuf].lpData, spt, LEN_BUF * sizeof(short));

        //추가적으로 파일저장
        //필터 tap3
        memcpy(in, header[iplaying == 0 ? NUM_BUF - 1 : iplaying - 1].lpData + 2 * (LEN_BUF - 3), 3 * sizeof(short));
        memcpy(in + 3, header[iplaying].lpData, LEN_BUF * sizeof(short));
        MovingAverageFilter(in, out, LEN_BUF, 3);
        fwrite(out, sizeof(short), LEN_BUF, fp3);
        //필터 tap5
        memcpy(in, header[iplaying == 0 ? NUM_BUF - 1 : iplaying - 1].lpData + 2 * (LEN_BUF - 5), 5 * sizeof(short));
        memcpy(in + 5, header[iplaying].lpData, LEN_BUF * sizeof(short));
        MovingAverageFilter(in, out, LEN_BUF, 5);
        fwrite(out, sizeof(short), LEN_BUF, fp5);
        //필터 tap9
        memcpy(in, header[iplaying == 0 ? NUM_BUF - 1 : iplaying - 1].lpData + 2 * (LEN_BUF - 9), 9 * sizeof(short));
        memcpy(in + 9, header[iplaying].lpData, LEN_BUF * sizeof(short));
        MovingAverageFilter(in, out, LEN_BUF, 9);
        fwrite(out, sizeof(short), LEN_BUF, fp9);
        //필터 없음
        fwrite(header[iplaying].lpData, sizeof(short), LEN_BUF, fpraw);


        if (filter_on == 1) {
            //필터에 입력되는 버퍼의 앞부분에 이전 버퍼 끝부분의 tap사이즈 만큼의 데이터 복사
            //이전 데이타를 이용해 음성이 끊어지지 않게 함. 
            //넣어주기 이전에는 n-tap만큼만 아웃풋 되게 되어 음성이 끊어졌었음
            memcpy(in, header[iplaying == 0 ? NUM_BUF - 1 : iplaying - 1].lpData + 2 * (LEN_BUF - filterTap), filterTap * sizeof(short));
            memcpy(in + filterTap, header[iplaying].lpData, LEN_BUF * sizeof(short));
        }
        else {
            memcpy(in, header[iplaying].lpData, LEN_BUF * sizeof(short));
        }
        //filterTap 변수에 따라 filter의 tap 변경
        if (filter_on == 1) {
            MovingAverageFilter(in, out, LEN_BUF, filterTap);
            memcpy(header[iplaying].lpData, out, LEN_BUF * sizeof(short));
        }
        fwrite(header[iplaying].lpData, sizeof(short), LEN_BUF, fp);



        result = waveOutPrepareHeader(hWaveOut, &header[iplaying], sizeof(WAVEHDR));
        result = waveOutWrite(hWaveOut, &header[iplaying], sizeof(WAVEHDR)); // play audio 
    }
    return 0;
}
DWORD WINAPI PlayingWaitingThread(LPVOID ivalue)
{
    while (1)
    {
        WaitForSingleObject(hEvent_FinishedPlaying, INFINITE);
        waveOutUnprepareHeader(hWaveOut, &header[iplaying], sizeof(WAVEHDR));
    }
}
static void CALLBACK waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
    if (uMsg != WOM_DONE)
        return;
    SetEvent(hEvent_FinishedPlaying);
}
void CALLBACK myWaveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
    if (uMsg != WIM_DATA)
        return;
    SetEvent(hEvent_BufferReady);
}
int main(int argc, TCHAR* argv[])
{
    hEvent_BufferReady = CreateEvent(NULL, FALSE, FALSE, NULL);
    hEvent_FinishedPlaying = CreateEvent(NULL, FALSE, FALSE, NULL);
    pFormat.wFormatTag = WAVE_FORMAT_PCM; // simple, uncompressed format 
    pFormat.nChannels = 1; // 1=mono, 2=stereo 
    pFormat.nSamplesPerSec = Samplerate;
    pFormat.wBitsPerSample = 16; // 16 for high quality, 8 for telephone-grade 
    pFormat.nBlockAlign = pFormat.nChannels * pFormat.wBitsPerSample / 8;
    pFormat.nAvgBytesPerSec = (pFormat.nSamplesPerSec) * (pFormat.nChannels) * (pFormat.wBitsPerSample) / 8;
    pFormat.cbSize = 0;
    short int* pBuf;
    size_t bpbuff = LEN_BUF;
    pBuf = new short int[bpbuff * NUM_BUF];
    result = waveInOpen(&hWaveIn, WAVE_MAPPER, &pFormat, (DWORD_PTR)myWaveInProc, 0L, CALLBACK_FUNCTION);
    result = waveOutOpen(&hWaveOut, WAVE_MAPPER, &pFormat, (DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION); // initialize all headers in the queue 
    for (int i = 0; i < NUM_BUF; i++) {
        header[i].lpData = (LPSTR)& pBuf[i * bpbuff];
        header[i].dwBufferLength = bpbuff * sizeof(*pBuf);
        header[i].dwFlags = 0L;
        header[i].dwLoops = 0L;
    }
    DWORD myThreadID;
    DWORD myThreadIDPlay;
    HANDLE hThread;
    HANDLE hThreadPlay;
    hThread = CreateThread(NULL, 0, RecordingWaitingThread, NULL, 0, &myThreadID);
    hThreadPlay = CreateThread(NULL, 0, PlayingWaitingThread, NULL, 0, &myThreadIDPlay);
    iBuf = 0;
    waveInPrepareHeader(hWaveIn, &header[iBuf], sizeof(WAVEHDR));
    waveInAddBuffer(hWaveIn, &header[iBuf], sizeof(WAVEHDR));
    waveInStart(hWaveIn);
    char key;

    printf("f : filter on \nr : filter off \ne : exit \n");
    do {
        key = _getch();
        if (key == 'f') {
            printf("Filter on\n");

            printf("enter tap : ");
            //scanf로 필터탭 변경 추가
            scanf("%d", &filterTap);
            filter_on = 1;
        }
        if (key == 'r') {
            printf("Filter off\n");
            filter_on = 0;
            filterTap = 0;
        }
        //센터 가중치 추가
        if (key == 'c') {
            if (center == 0) {
                printf("중간값에 가중치를 켬\n");
                center = 1;
            }
            else {
                printf("중간값에 가중치를 끔\n");
                center = 0;
            }
        }
    } while (key != 'e');
    fclose(fp);

    waveInClose(hWaveIn);
    waveOutClose(hWaveOut);
    CloseHandle(hThread);
    CloseHandle(hThreadPlay);
    CloseHandle(hEvent_BufferReady);
    CloseHandle(hEvent_FinishedPlaying);
    return 0;
}


