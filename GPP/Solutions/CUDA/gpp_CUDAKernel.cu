#include "../../../ComplexClass/CustomComplex.h"

__global__ void gpp_2D_CUDAKernel(int number_bands, int ngpown, int ncouls, \
    device_Array1D<dataType> achtemp_re, device_Array1D<dataType> achtemp_im, \
    device_Array1D<int> inv_igp_index, device_Array1D<int> indinv, device_Array1D<dataType> wx_array, \
        device_Array2D<CustomComplex<dataType>> aqsmtemp, device_Array2D<CustomComplex<dataType>> aqsntemp, \
        device_Array1D<dataType> vcoul, device_Array2D<CustomComplex<dataType>> wtilde_array, \
        device_Array2D<CustomComplex<dataType>> I_eps_array)
{
    dataType achtemp_re_loc[nend-nstart], achtemp_im_loc[nend-nstart];
    for(int iw = nstart; iw < nend; ++iw) {achtemp_re_loc[iw] = 0.00; achtemp_im_loc[iw] = 0.00;}

    for(int n1 = blockIdx.x; n1<number_bands; n1+=gridDim.x)
    {
        for(int my_igp=blockIdx.y; my_igp<ngpown; my_igp+=gridDim.y)
        {
            const int indigp = inv_igp_index(my_igp);
            const int igp = indinv(indigp);
            CustomComplex<dataType> sch_store1 = CustomComplex_conj(aqsmtemp(n1,igp))*  aqsntemp(n1,igp) * 0.5 * vcoul(igp);

            for(int ig = threadIdx.x; ig<ncouls; ig+=blockDim.x)
            {
                for(int iw = nstart; iw < nend; ++iw)
                {
                    CustomComplex<dataType> wdiff = wx_array(iw) - wtilde_array(my_igp,ig);
                    double wdiff_r = CustomComplex_real(wdiff * CustomComplex_conj(wdiff));
                    wdiff_r = 1/wdiff_r;

                    CustomComplex<dataType> delw = wtilde_array(my_igp, ig) * CustomComplex_conj(wdiff) * wdiff_r;
                    CustomComplex<dataType> sch_array = sch_store1 * I_eps_array(my_igp,ig) * delw ;

                    achtemp_re_loc[iw] += CustomComplex_real(sch_array);
                    achtemp_im_loc[iw] += CustomComplex_imag(sch_array);
                }
            }
        } //ngpown
    } //number_bands

    //Add the final results here;
    for(int iw = nstart; iw < nend; ++iw)
    {
        atomicAdd(&achtemp_re(iw), achtemp_re_loc[iw]);
        atomicAdd(&achtemp_im(iw), achtemp_im_loc[iw]);
    }
}

void noflagOCC_cudaKernel(int number_bands, int ngpown, int ncouls, \
        device_Array1D<dataType> achtemp_re, device_Array1D<dataType> achtemp_im, \
        device_Array1D<int> inv_igp_index, device_Array1D<int> indinv, device_Array1D<dataType> wx_array, \
        device_Array2D<CustomComplex<dataType>> aqsmtemp, device_Array2D<CustomComplex<dataType>> aqsntemp, \
        device_Array1D<dataType> vcoul, device_Array2D<CustomComplex<dataType>> wtilde_array, \
        device_Array2D<CustomComplex<dataType>> I_eps_array)

{
    dim3 numBlocks(number_bands, ngpown);
    dim3 numThreads(32,1,1);
    printf("Launching a double dimension grid with numBlocks = (%d, %d) and %d threadsPerBlock \n", number_bands, ngpown, numThreads.x);

    gpp_2D_CUDAKernel<<<numBlocks, numThreads>>>(number_bands, ngpown, ncouls, achtemp_re, achtemp_im, \
            inv_igp_index, indinv, wx_array, \
        aqsmtemp, aqsntemp, vcoul, wtilde_array, I_eps_array);
}


