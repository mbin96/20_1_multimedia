
#include <minmax.h> 
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
short in[LEN_BUF + 30], out[LEN_BUF], out_chord[LEN_BUF];
char filter_on = 0;
HWAVEIN hWaveIn;
HWAVEOUT hWaveOut;
WAVEFORMATEX pFormat;


//fr filter
#define PI 3.141592
#define OFF 0
#define MOV_AVG 1   
#define FIR 2 
#define IIR 3
#define RECT 0 
#define HANN 1 
#define HAMM 2 
#define TRIA 3
float fc, * fir_coef;
int fir_tap, w_type;
//frfilter end

//karaoke
#define KEY 4
#define NUM_KEYS 5

int cur_key,dir;
double key_freq[] = { 10000, 330, 349, 392, 523 };
double phs, cur_corl; 
char key_name[][5] = { "C4","E4","F4","G4", "C5"}; 
short* key_wav;


void Build_Sinewaves(int key_id, short* d, int N) {
    double   div_f; 
    int div_i;
    for (int i = 0; i < N; i++) 
        d[i] = (short)(6000 * sin(2 * PI * 
            (key_freq[key_id] / Samplerate) * i + phs));

    phs = 2 * PI * (key_freq[key_id] / Samplerate) * N + phs;
    div_f = phs / (PI * 2); 
    div_i = (int)div_f; 
    phs = phs - div_i * PI * 2;
}

double Correlation(short *x, short *y, int N, int key_id)
{
    int max_lag, lag, i, j;
    double mx, my, sx, sy, corl, max_corl;
    max_lag = Samplerate / key_freq[key_id];

    for (i = 0, max_corl = 0; i < max_lag; i++)
    {
        corl = 0;
        mx = my = sx = sy = 0.;
        for (j = 0; j < N - i; j++)
        {
            mx += x[j];
            my += y[j + i];
            sx += x[j] * x[j];
            sy += y[j + i] * y[j + i];
            corl += x[j] * y[j + i];
        }
        mx = mx / (N - i);
        my = my / (N - i);
        sx = sqrt(sx / (N - i) - mx * mx);
        sy = sqrt(sy / (N - i) - my * my);
        corl = (corl / (N - i) - mx * my) / (sx * sy);
        corl = fabs(corl);
        max_corl = (corl > max_corl) ? corl : max_corl;
    }

    return max_corl;
}

//하나의 음성데이터에 각각 다른 필터를 적용하여 저장하기
FILE* fp = fopen("result_.snd", "wb");//출력되는 음성
FILE* fpraw = fopen("result_raw_.raw", "wb");//필터안된음성
// FILE* fp3 = fopen("result_tap3_.raw", "wb");//tpa3 필터음성
// FILE* fp5 = fopen("result_tap5_.raw", "wb");//tap5 필터음선
// FILE* fp9 = fopen("result_tap9_.raw", "wb");// tap9 필터음성


//fr filter 
void Make_Win(float* w, int len, int w_type)
{
    int i;
    for (i = 0; i < len; i++)
    {
        if (w_type == RECT)
            w[i] = 1;
        if (w_type == HANN)
            w[i] = (float)(0.5 * (1 - cos(2 * 3.141592 * i / (len - 1.))));
        if (w_type == HAMM)
            w[i] = 0.54 - 0.46 * cos(3.141592 * 2. * i / (len - 1));
        if (w_type == TRIA)
            w[i] = (i < len / 2) ? ((float)i * 2) / len : 2 * (1 - (float)i / len);
    }
}



int iir_order;
float * iir_coefA, * iir_coefB;


void IIR_filter(short* in, short* out, int N, int iir_order)
{
    int i, j;
    float v;
    for (i = 0; i < N; i++)
    {
        v = 0;
        for (j = 0; j < iir_order; j++)
        {
            if (i - j >= 0)
            {
                v += iir_coefB[j] * in[i - j];
                v += iir_coefA[j] * out[i - j];
            }
        }
        out[i] = (short)v;
    }
}

void GetFIRcoefs(float* coef, float fc, int flen, int w_type)
{
    //flen == tap
    int n, t;
    float* w, f_norm;
    w = (float*)malloc(sizeof(float) * flen);
    Make_Win(w, flen, w_type);
    f_norm = (float)fc / Samplerate;
    for (n = 0; n < flen; n++)
    {
        t = (n - flen / 2);
        if (t == 0)
            coef[n] = w[n] * 2 * f_norm;
        else
            coef[n] = w[n] * sin(2 * 3.141592 * f_norm * t) / (3.141592 * t);
    }
    free(w);
}

void GetHPFcoefs(float* FIRcoefs, int tap)
{
    for (int i = 0; i < tap; i++)
    {
        if (i % 2 == 1)
            FIRcoefs[i] = -FIRcoefs[i];
    }
}

void GetBPFcoefs(float* FIRcoefs, int tap, float passFc)
{
    for (int i = 0; i < tap; i++)
    {
        int t = (i - tap / 2);
        //if (i % 2 == 1)
        FIRcoefs[i] = FIRcoefs[i] * 2* cos(2*PI* passFc *t / Samplerate);
    }
}

//input BPFcoefs
void GetBSFcoefs(float* BPFcoef, int tap)
{
    for (int i = 0; i < tap; i++)
    {
        if (i == (int)(tap+1)/2)
            BPFcoef[i] = 1.0 - BPFcoef[i];
            
        else    
            BPFcoef[i] = 0-BPFcoef[i];
    }
}

//in - inbuffer out - outputbuffer N - samplingrate tap 
void FIR_filter(short* in, short* out, float* coef, int N, int tap)
{
    int i, j, k;
    float v;
    for (i = 0; i < N; i++)
    {
        for (j = 0, v = 0; j < tap; j++)
        {
            k = i - tap / 2 + j;
            if (0 <= k && k < N)
                v += in[k] * coef[j];
        }
        out[i] = (short)v;
    }
}
#define HIGH 1


//



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
    // for (i = 0; i < N + tap; i++)
     for (i = 0; i < N ; i++)
    {
        for (j = 0, t = 0, M = 0; j < tap; j++)
        {
            if (i - j >= 0) {
                // //가중치 여부
                // //가중치 변수 center가 1인경우 가운데 값을 2배로 더하여줌
                // if (center == 1) {
                //     if (j == (int)(tap / 2)) {
                //         t += 2 * in[i - j];
                //         M++;
                //     }
                //     else {
                //         t += in[i - j];
                //     }
                //     M++;
                // }
                // else {
                t += in[i - j];
                M++;
                // }

            }
        }
        // if (tap <= i)
            // out[i - tap] = (short)(t / M);
        out[i] = (short)(t / M);
    
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

        // //추가적으로 파일저장
        // //필터 tap3
        // memcpy(in, header[iplaying == 0 ? NUM_BUF - 1 : iplaying - 1].lpData + 2 * (LEN_BUF - 3), 3 * sizeof(short));
        // memcpy(in + 3, header[iplaying].lpData, LEN_BUF * sizeof(short));
        // MovingAverageFilter(in, out, LEN_BUF, 3);
        // fwrite(out, sizeof(short), LEN_BUF, fp3);
        // //필터 tap5
        // memcpy(in, header[iplaying == 0 ? NUM_BUF - 1 : iplaying - 1].lpData + 2 * (LEN_BUF - 5), 5 * sizeof(short));
        // memcpy(in + 5, header[iplaying].lpData, LEN_BUF * sizeof(short));
        // MovingAverageFilter(in, out, LEN_BUF, 5);
        // fwrite(out, sizeof(short), LEN_BUF, fp5);
        // //필터 tap9
        // memcpy(in, header[iplaying == 0 ? NUM_BUF - 1 : iplaying - 1].lpData + 2 * (LEN_BUF - 9), 9 * sizeof(short));
        // memcpy(in + 9, header[iplaying].lpData, LEN_BUF * sizeof(short));
        // MovingAverageFilter(in, out, LEN_BUF, 9);
        // fwrite(out, sizeof(short), LEN_BUF, fp9);
        //필터 없음
        fwrite(header[iplaying].lpData, sizeof(short), LEN_BUF, fpraw);

        
        // if (filter_on == 1) {
        //     //필터에 입력되는 버퍼의 앞부분에 이전 버퍼 끝부분의 tap사이즈 만큼의 데이터 복사
        //     //이전 데이타를 이용해 음성이 끊어지지 않게 함. 
        //     //넣어주기 이전에는 n-tap만큼만 아웃풋 되게 되어 음성이 끊어졌었음
        //     memcpy(in, header[iplaying == 0 ? NUM_BUF - 1 : iplaying - 1].lpData + 2 * (LEN_BUF - filterTap), filterTap * sizeof(short));
        //     memcpy(in + filterTap, header[iplaying].lpData, LEN_BUF * sizeof(short));
        // }
        // else {
            memcpy(in, header[iplaying].lpData, LEN_BUF * sizeof(short));
        // }

        switch(filter_on){
            case OFF : break;
            case MOV_AVG : {
                MovingAverageFilter(in, out, LEN_BUF, 5); 
                memcpy(header[iplaying].lpData, out, LEN_BUF * sizeof(short));
                break;
            }
            case FIR : {
                FIR_filter(in, out, fir_coef, Samplerate, fir_tap);
                memcpy(header[iplaying].lpData, out, LEN_BUF * sizeof(short));
                break;
            }

            case IIR : {
                IIR_filter(in, out, Samplerate, iir_order);
                memcpy(header[iplaying].lpData, out, LEN_BUF * sizeof(short));
                break;
            }
            case KEY :{
                Build_Sinewaves(cur_key, out, LEN_BUF);
                
                //아래코드는 화음만들때 사용
                //Build_Sinewaves(cur_key+1, out_chord, LEN_BUF);
                //for (int i = 0; i < LEN_BUF; i++) {
                //    out [i] = out_chord[i] + out[i];
                //}

                cur_corl = Correlation(in, out, LEN_BUF, cur_key);

                printf("현재 음정 [%s] 와 상관값: %f\n", key_name[cur_key], cur_corl);
                memcpy(header[iplaying].lpData, out, LEN_BUF * sizeof(short));

            }
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

    /*
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
    */
    int fq_BP;
    do
    {
        printf("======selmode======\n");
        printf("m. movingavg\nf. FIR\n");
        if (filter_on == FIR) {
            printf("k.karaoke \nc. set cutoff freq\nt. set tap of filter\nw. set window type\np. stop or pass\n");
        }
        printf("r. filter off\ne. exit\n");

        key = getch();
        if (key == 'm')
        {
            printf("\nMoving average filter on\n");
            filter_on = MOV_AVG;
        }else if (key == 'k'){
            filter_on = 0;
            printf("\n음정 연습\n");
            printf("키이를 입력하세요. \n");
            for (int i = 0; i < NUM_KEYS; i++)
                printf(" %s(%d)", key_name[i], i);
            printf("...");
            scanf("%d", &cur_key);
            filter_on = KEY;
        }
        else if (key == 'i') {
            printf("\nIIR filter on\n");
            printf("Enter order : ");
            scanf("%d", &iir_order);
            if (iir_coefA != NULL) {
                free(iir_coefA);
            }
            if (iir_coefB != NULL) {
                free(iir_coefB);
            }
            iir_coefA = (float*)malloc(sizeof(float) * iir_order);
            iir_coefB = (float*)malloc(sizeof(float) * iir_order);
            printf("Enter coef: \n");
            for (int i = 0; i < iir_order; i++)
            {
                printf("A[%d], B[%d] : ", i, i);
                scanf("%f %f", &iir_coefA[i], &iir_coefB[i]);
            }
            filter_on = IIR;
        }
        if (key == 'f')
        {
            printf("\nFIR filter on\n");
            printf("Cutoff freqency (in Hz) : ");
            scanf("%f", &fc);
            printf("Filter length (in samples, should be odd number) : ");
            scanf("%d", &fir_tap);
            printf("Window type (0=Rect/1=Hann/2=Hamm/3=Tria) ");
            scanf("%d", &w_type);
            if (fir_coef != NULL)
                free(fir_coef);
            fir_coef = (float*)malloc(sizeof(float) * fir_tap);
            GetFIRcoefs(fir_coef, fc, fir_tap, w_type);
            printf("(L)PF or (H)PF ");
            key = getch();
            if (key == 'H' || key == 'h')
            {
                printf("\nHPF selected.\n");
                GetHPFcoefs(fir_coef, fir_tap);
            }
            else
                printf("\nLPF selected.\n");
            filter_on = FIR;
        }
        if (key == 'c') {
            printf("Cutoff freqency (in Hz) : ");
            scanf("%f", &fc);
            if (fir_coef != NULL)
                free(fir_coef);
            fir_coef = (float*)malloc(sizeof(float) * fir_tap);
            GetFIRcoefs(fir_coef, fc, fir_tap, w_type);
        }if (key == 't') {
            printf("Filter length (in samples, should be odd number) : ");
            scanf("%d", &fir_tap);
            if (fir_coef != NULL)
                free(fir_coef);
            fir_coef = (float*)malloc(sizeof(float) * fir_tap);
            GetFIRcoefs(fir_coef, fc, fir_tap, w_type);
        }if (key == 'w') {
            printf("Window type (0=Rect/1=Hann/2=Hamm/3=Tria) ");
            scanf("%d", &w_type);
            if (fir_coef != NULL)
                free(fir_coef);
            fir_coef = (float*)malloc(sizeof(float) * fir_tap);
            GetFIRcoefs(fir_coef, fc, fir_tap, w_type);
        }if (key == 'p') {
            printf("(H)ighpass, (L)owpass, band(P)ass, band(S)top >");
            key = getch();
                
            if (fir_coef != NULL)
                free(fir_coef);
            fir_coef = (float*)malloc(sizeof(float) * fir_tap);
            GetFIRcoefs(fir_coef, fc, fir_tap, w_type);
            switch (key) {
            case 'h' : 
                GetHPFcoefs(fir_coef, fir_tap);
                break;
            case 'p' :
                printf("input bandpass fq >");
                
                scanf("%d", &fq_BP);
                GetBPFcoefs(fir_coef, fir_tap, fq_BP);
                break;

            case 's':
                printf("input bandstop fq >");
                scanf("%d", &fq_BP);
                GetBPFcoefs(fir_coef, fir_tap, fq_BP);
                GetBSFcoefs(fir_coef, fir_tap);
                break;
            case 'l' :
                break;
            }
            
        }
        if (key == 'r')
        {
            printf("Filter off\n");
            filter_on = 0;
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