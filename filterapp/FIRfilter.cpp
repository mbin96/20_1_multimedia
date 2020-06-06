#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include <conio.h>

#pragma warning(disable : 4996)

#pragma comment(lib, "winmm.lib") 

#include<string>
#include<iostream>
#include<ctime>
#include<fstream>

#define Samplerate 16000 
#define PI 3.141592
//fr filter
#define OFF 0
#define MOV_AVG 1   
#define FIR 2 
#define IIR 3
//FIRfilter window
#define RECT 0 
#define HANN 1 
#define HAMM 2 
#define TRIA 3

//lowpassfilter
#define LPF 0 
//highpass filter
#define HPF 1 
//bandpassfilter
#define BPF 2 
//bandstopfilter
#define BSF 3

using namespace std;



class filter {
private:
    int Filter = FIR;
    int whatWindow = RECT;
    int whatPass = LPF;
    int tap = 29;
    float* coef;
    float* window;
    float fc = 1000;
    float passFc = 1000;

    void Make_Win();
    void GetFIRcoefs();
    void GetHPFcoefs();
    void GetBPFcoefs();
    void GetBSFcoefs();
    void getCoef();
    //void FIR_filter(short* in, short* out, int bufferSize);
public:
    void setWindow(int kind);
    void setFilter(int kind);
    void setTap(int tap);
    void setPassFreq(int freq);
    void setFilterFreq(int freq);
    void FIR_filter(short * in, short * out, int bufferSize);
    string getSet();
};
string filter::getSet() {
    string result;
    result = "out";
    
    switch (whatPass) {
    case LPF:
        result += "_LPF";
        break;
    case HPF:
        result += "_HPF";
        break;
    case BPF:
        result += "_BPF";
        break;
    case BSF:
        result += "_BSF";
        break;
    }

    switch (whatWindow) {
    case RECT:
        result += "_RECT";
        break;
    case HANN:
        result += "_HANN";
        break;
    case HAMM:
        result += "_HAMM";
        break;
    case TRIA:
        result += "_TRIA";
        break;
    }
    result  += "_fq"+ std::to_string(fc) + "_BPfq" + std::to_string(passFc) +"_tap" + std::to_string(tap)+ "time" + std::to_string(time(0)) + ".raw";
    return result;
}

void filter::setWindow(int kind) {
    whatWindow = kind;
    getCoef();
}
void filter::setFilter(int kind) {
    whatPass = kind;
    getCoef();
}
void filter::setTap(int tapsize) {
    tap = tapsize;
    getCoef();
}
void filter::setPassFreq(int freq) {
    passFc = freq;
    getCoef();
}
void filter::setFilterFreq(int freq) {
    fc = (float)freq;
    getCoef();
}


void filter::getCoef() {
    switch (whatPass) {
    case LPF:
        GetFIRcoefs();
        break;
    case HPF:
        GetHPFcoefs();
        break;
    case BPF:
        GetBPFcoefs();
        break;
    case BSF:
        GetBSFcoefs();
        break;
    }
}
    
void filter::Make_Win()
{
    int i;
    for (i = 0; i < tap; i++)
    {
        if (whatWindow == RECT)
            window[i] = 1;
        if (whatWindow == HANN)
            window[i] = (float)(0.5 * (1 - cos(2 * 3.141592 * i / (tap - 1.))));
        if (whatWindow == HAMM)
            window[i] = 0.54 - 0.46 * cos(3.141592 * 2. * i / (tap - 1));
        if (whatWindow == TRIA)
            window[i] = (i < tap / 2) ? ((float)i * 2) / tap : 2 * (1 - (float)i / tap);
    }
}



void filter::GetFIRcoefs()
{
    //flen == tap
    int n, t;
    float f_norm;
    window = (float*)malloc(sizeof(float) * tap);
    Make_Win();

    if (coef != NULL)
        free(coef);
    coef = (float*)malloc(sizeof(float) * tap);

    f_norm = (float)fc / Samplerate;
    for (n = 0; n < tap; n++)
    {
        t = (n - tap / 2);
        if (t == 0)
            coef[n] = window[n] * 2 * f_norm;
        else
            coef[n] = window[n] * sin(2 * 3.141592 * f_norm * t) / (3.141592 * t);
    }
    free(window);
}

void filter::GetHPFcoefs()
{
    GetFIRcoefs();
    for (int i = 0; i < tap; i++)
    {
        if (i % 2 == 1)
            coef[i] = -coef[i];
    }
}

void filter::GetBPFcoefs()
{
    GetFIRcoefs();
    for (int i = 0; i < tap; i++)
    {
        //if (i % 2 == 1)
        coef[i] = coef[i] * 2 * cos(2 * PI * passFc * i / Samplerate);
    }
}

//input BPFcoefs
void filter::GetBSFcoefs()
{
    GetBPFcoefs();
    for (int i = 0; i < tap; i++)
    {
        if (i == (int)(tap + 1) / 2)
            coef[i] = 1.0 - coef[i];
        else
            coef[i] = 0 - coef[i];
    }
}

//in - inbuffer out - outputbuffer N - samplingrate tap 
void filter::FIR_filter(short* in, short* out, int bufferSize)
{
    int i, j, k;
    float v;
    for (i = 0; i < bufferSize; i++)
    {
        for (j = 0, v = 0; j < tap; j++)
        {
            k = i - tap / 2 + j;
            if (0 <= k && k < bufferSize)
                v += in[k] * coef[j];
        }
        out[i] = (short)v;
    }
}


void MovingAverageFilter(short* in, short* out, int N, int tap) {
    int i, j, M;
    float t;
    //N+tap으로 변경
    //이전 버퍼의 데이터를 앞에 넣어줬기때문에 끊어지지 않는 녹음 가능
    // for (i = 0; i < N + tap; i++)
    for (i = 0; i < N; i++)
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






int main(int argc, char** argv) {
    ifstream fin;
    fin.open(argv[1], ios::in | ios::binary);
    ofstream fout;

    fin.seekg(0, ios::end);
    int fileSize = fin.tellg();
    fin.seekg(0, ios::beg);

    char* buf, * outbuf;
    buf = new char[fileSize];
    outbuf = new char[fileSize];

    fin.read(buf, fileSize);

    filter firFilter;


    void setWindow(int kind);
    void setFilter(int kind);
    void setTap(int tap);
    void setPassFreq(int freq);
    void setFilterFreq(int freq);
    
    char key;
    int input;
    do
    {
        printf("======selmode======\n");
        printf("m. movingavg\nf. FIR\n");
        printf("c. set cutoff freq\nt. set tap of filter\nw. set window type\np. stop or pass\ns. save\n");
        printf("r. filter off\ne. exit\n");
        key = getch();

        if (key == 'f')
        {
            printf("\nFIR filter on\n");

            printf("Cutoff freqency (in Hz) : ");
            scanf("%d", &input);
            firFilter.setFilterFreq(input);
            
            printf("Filter length (in samples, should be odd number) : ");
            scanf("%d", &input);
            firFilter.setTap(input);
            
            printf("Window type (0=Rect/1=Hann/2=Hamm/3=Tria) ");
            scanf("%d", &input);
            firFilter.setWindow(input);
            
            printf("(H)ighpass, (L)owpass, band(P)ass, band(S)top >");
            key = getch();
            switch (key) {
            case 'h' :
                firFilter.setFilter(HPF);
                break;
            case 'l':
                firFilter.setFilter(LPF);
                break;
            case 'p':
                firFilter.setFilter(BPF);
                printf("input bandpass fq >");
                scanf("%d", &input);
                firFilter.setPassFreq(input);
                break;
            case 's':
                firFilter.setFilter(BSF);
                printf("input bandpass fq >");
                scanf("%d", &input);
                firFilter.setPassFreq(input);
                break;
            }
            
        }
        if (key == 'c') {
            printf("Cutoff freqency (in Hz) : ");
            scanf("%d", &input);
            firFilter.setFilterFreq(input);

        }if (key == 't') {
            printf("Filter length (in samples, should be odd number) : ");
            scanf("%d", &input);
            firFilter.setTap(input);

        }if (key == 'w') {
            printf("Window type (0=Rect/1=Hann/2=Hamm/3=Tria) ");
            scanf("%d", &input);
            firFilter.setWindow(input);

        }if (key == 'p') {
            printf("(H)ighpass, (L)owpass, band(P)ass, band(S)top >");
            key = getch();
            switch (key) {
            case 'h':
                firFilter.setFilter(HPF);
                break;
            case 'l':
                firFilter.setFilter(LPF);
                break;
            case 'p':
                firFilter.setFilter(BPF);
                printf("input bandpass fq >");
                scanf("%d", &input);
                firFilter.setPassFreq(input);
                break;
            case 's':
                firFilter.setFilter(BSF);
                printf("input bandpass fq >");
                scanf("%d", &input);
                firFilter.setPassFreq(input);
                break;
            }

        }else if (key == 's')
        {
            firFilter.FIR_filter((short*)buf, (short*)outbuf, fileSize/2);
            fout.open(firFilter.getSet(), ios::out | ios::binary);
            fout.write(outbuf, fileSize);
            fout.close();
           
        }
    } while (key != 'e');
    fin.close();
    //firFilter.setFilter(LPF);

    

}