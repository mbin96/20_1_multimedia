#include <stdio.h>
#include <stdlib.h> 
#include ".\\fftw3.h"
#include ".\\dft\\dft.h"
#include <time.h> 
#include <math.h> 
#pragma comment(lib, ".\\fftw\\libfftw3-3.lib") 
#pragma warning(disable : 4996)

int main(int argc, char **argv)
{

    while(1){
    fftw_complex *data, *fft_result, *ifft_result;
    fftw_plan plan_forward, plan_backward;
    int i, F_SIZE;
    COMPLEX *ft_in, *ft_out;
    clock_t start, end;
    printf("Enter FFT SIZE : ");
    scanf("%d", &F_SIZE);
    ft_in = (COMPLEX *)malloc(sizeof(COMPLEX) * F_SIZE);
    ft_out = (COMPLEX *)malloc(sizeof(COMPLEX) * F_SIZE);


    for (i = 0; i < F_SIZE; i++)
    {
        ft_in[i].real = i;
        ft_in[i].imag = i * 0.5;
    }


    start = clock();
    for (i = 0; i < 100; i++)
    {
        dft(ft_in, ft_out, F_SIZE);
        idft(ft_out, ft_in, F_SIZE);
    }
    end = clock();
    // for (i = 0; i < F_SIZE; i++)
    // {
    //     printf("DFT %d > %.4f %.4f\n", i, ft_in[i].real, ft_in[i].imag);
    // }
    printf("[DFT]Eclipse time = %6.3f sec.\n", (end - start) * 0.001);

    for (i = 0; i < F_SIZE; i++)
    {
        ft_in[i].real = i;
        ft_in[i].imag = i * 0.5;
    }

    start = clock();
    for (i = 0; i < 100; i++)
    {
        fft(ft_in, log2((unsigned)F_SIZE));
        ifft(ft_in, log2((unsigned)F_SIZE));
    }
    end = clock();

    // for (i = 0; i < F_SIZE; i++)
    // {
    //     printf("FFT %d > %.4f %.4f\n", i, ft_in[i].real, ft_in[i].imag);
    // }
    printf("[FFT]Eclipse time = %6.3f sec.\n", (end - start) * 0.001);


    data = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * F_SIZE);
    fft_result = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * F_SIZE);
    ifft_result = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * F_SIZE);
    plan_forward = fftw_plan_dft_1d(F_SIZE, data, fft_result, FFTW_FORWARD, FFTW_ESTIMATE);
    plan_backward = fftw_plan_dft_1d(F_SIZE, fft_result, ifft_result, FFTW_BACKWARD, FFTW_ESTIMATE);
    
    for (i = 0; i < F_SIZE; i++)
    {
        data[i][0] = i;
        data[i][1] = i * 0.5;
    }
    
    start = clock();
    
    for (i = 0; i < 100; i++)
    {
        fftw_execute(plan_forward); 
        fftw_execute(plan_backward);
    }
    
    end = clock();
    
    for (i = 0; i < F_SIZE; i++)
    {
        printf("FFTW %d > %.4f %.4f\n", i, (float)ifft_result[i][0]*pow(2,-10), (float) ifft_result[i][1] * pow(2, -10));
    }
    //printf("[FFT]Eclipse time = %6.3f sec.\n", (end - start) * 0.001);
    printf("[FFTW]Eclipse time = %6.3f sec.\n", (end - start) * 0.001);
    fftw_destroy_plan(plan_forward);
    fftw_destroy_plan(plan_backward);
    fftw_free(data);
    fftw_free(fft_result);
    fftw_free(ifft_result);
 

    // fftw_complex *data, *fft_result, *ifft_result;
    // fftw_plan plan_forward, plan_backward;
    // int i, F_SIZE;
    // printf("Enter FFT SIZE : ");
    // clock_t start, end;
    // scanf("%d", &F_SIZE);

    // data = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * F_SIZE);
    // fft_result = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * F_SIZE);
    // ifft_result = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * F_SIZE);

    // plan_forward = fftw_plan_dft_1d(F_SIZE, data, fft_result, FFTW_FORWARD, FFTW_ESTIMATE);
    // plan_backward = fftw_plan_dft_1d(F_SIZE, fft_result, ifft_result, FFTW_BACKWARD, FFTW_ESTIMATE);
    
    // for (i = 0; i < F_SIZE; i++)
    // {
    //     data[i][0] = i;
    //     data[i][1] = i * 0.5;
    // }

    // start = clock();
    // fftw_execute(plan_forward);
    // end = clock();
    // float sum = 0;
    // for (i = 0; i < F_SIZE; i++)
    // {
    //     sum += pow(data[i][0] - fft_result[i][0], 2);
    //     sum += pow(data[i][1] - fft_result[i][1], 2);
    // }
    // printf("%f\n", sum);
    // fftw_execute(plan_backward);

    // sum = 0;
    // for (i = 0; i < F_SIZE; i++)
    // {
    //     sum += ifft_result[i][0] - i;
    //     sum += ifft_result[i][1] - i * 0.5;
    // }
    // printf("%f\n", sum);
    

    // printf("%f\n", (float)(end - start));


    // fftw_destroy_plan(plan_forward);
    // fftw_destroy_plan(plan_backward);

    // fftw_free(data);
    // fftw_free(fft_result);
    // fftw_free(ifft_result);


    // COMPLEX* ft_in, * ft_out, * orig;

    // ft_in = (COMPLEX*)malloc(sizeof(COMPLEX) * F_SIZE);
    // ft_out = (COMPLEX*)malloc(sizeof(COMPLEX) * F_SIZE);
    // orig = (COMPLEX*)malloc(sizeof(COMPLEX) * F_SIZE);
    // for (i = 0; i < F_SIZE; i++)
    // {
    //     ft_in[i].real = i;
    //     ft_in[i].imag = i * 0.5;
    //     orig[i].real = i;
    //     orig[i].imag = i * 0.5;
    // }
    
    // start = clock();
    // dft(ft_in, ft_out, F_SIZE);
    // end = clock();
    // printf("%f\n", (float)(end-start));
    // idft(ft_out, ft_in, F_SIZE);

    // sum = 0;
    // for (i = 0; i < F_SIZE; i++)
    // {
    //     ft_in[i].real = i;
    //     ft_in[i].imag = i * 0.5;
    //     sum += ft_in[i].real - orig[i].real;
    //     sum += ft_in[i].real - orig[i].imag;
    // }
    // printf("%f", sum);


    // fft(ft_in, log2((unsigned)F_SIZE));
    // ifft(ft_in, log2((unsigned)F_SIZE));



    }
    // return 0;
}

// int main(int argc, char** argv)
// {
//     int i, F_SIZE;
//     COMPLEX* ft_in, * ft_out;
//     clock_t start, end;
//     printf("Enter FFT SIZE : ");
//     scanf("%d", &F_SIZE);
//     ft_in = (COMPLEX*)malloc(sizeof(COMPLEX) * F_SIZE);
//     ft_out = (COMPLEX*)malloc(sizeof(COMPLEX) * F_SIZE);
//     for (i = 0; i < F_SIZE; i++)
//     {
//         ft_in[i].real = i;
//         ft_in[i].imag = i * 0.5;
//     }
//     start = clock();
//     dft(ft_in, ft_out, F_SIZE);
//     end = clock();
//     printf("%f\n", (float)(end-start) / CLOCKS_PER_SEC);
//     idft(ft_out, ft_in, F_SIZE);
//     fft(ft_in, log2((unsigned)F_SIZE));
//     ifft(ft_in, log2((unsigned)F_SIZE));
// }