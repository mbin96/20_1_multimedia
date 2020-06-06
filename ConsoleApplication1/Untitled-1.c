void Make_Win(float *w, int len, int w_type)
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

void GetFIRcoefs(float *coef, float fc, int flen, int w_type)
{
    int n, t;
    float *w, f_norm;
    w = (float *)malloc(sizeof(float) * flen);
    Make_Win(w, flen, w_type);
    f_norm = (float)fc / Samplerate;
    for (n = 0; n < flen; n++)
    {
        t = (n – flen / 2);
        if (t == 0)
            coef[n] = w[n] * 2 * f_norm;
        else
            coef[n] = w[n] * sin(2 * 3.141592 * f_norm * t) / (3.141592 * t);
    }
    free(w);
}

void GetHPFcoefs(float *coef, int tap)
{
    for (int i = 0; i < tap; i++)
    {
        if (i % 2 == 1)
            coef[i] = -coef[i];
    }
}
Void FIR_filter(short *in, short *out, float *coef, int N, int tap)
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

do
{
    key = getch();
    if (key == 'm')
    {
        printf("\nMoving average filter on\n");
        filter_on = MOV_AVG;
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
        fir_coef = (float *)malloc(sizeof(float) * fir_tap);
        GetFIRcoefs(fir_coef, fc, fir_tap, w_type);
        printf("(L)PF or (H)PF ");
        key = getche();
        if (key == ‘H’|| key == ‘h’)
        {
            printf("\nHPF selected.\n");
            GetHPFcoefs(fir_coef, fir_tap);
        }
        else
            printf("\nLPF selected.\n");
        filter_on = FIR;
    }
    if (key == 'r')
    {
        printf("Filter off\n");
        filter_on = 0;
    }
} while (key != 'e');

void MovingAverageFilter(short* in, short* out, int N, int tap) {
    int i, j, M;
    float t;
    for (i = 0; i < N; i++) { for (j = 0, t = 0, M = 0; j < tap; j++) { if (i - j >= 0) { t += in[i - j]; M++; } } out