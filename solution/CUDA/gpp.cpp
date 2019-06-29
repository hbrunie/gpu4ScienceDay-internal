#include "../../external/ComplexClass/CustomComplex.h"
#include "../../external/arrayMD/arrayMDcpu.h"
#include "../../external/arrayMD/arrayMDgpu.h"
#include <cuda.h>

void noflagOCC_cudaKernel(int number_bands, int ngpown, int ncouls, \
        device_Array1D<dataType> achtemp_re, device_Array1D<dataType> achtemp_im, device_Array1D<int> inv_igp_index, device_Array1D<int> indinv, device_Array1D<dataType> wx_array, \
        device_Array2D<CustomComplex<dataType>> aqsmtemp, device_Array2D<CustomComplex<dataType>> aqsntemp, \
        device_Array1D<dataType> vcoul, device_Array2D<CustomComplex<dataType>> wtilde_array, \
        device_Array2D<CustomComplex<dataType>> I_eps_array);

void noflagOCC_solver(size_t number_bands, size_t ngpown, size_t ncouls, Array1D<int> &inv_igp_index, Array1D<int> &indinv, Array1D<dataType>& wx_array, Array2D<CustomComplex<dataType>>& wtilde_array, \
        Array2D<CustomComplex<dataType>> &aqsmtemp, Array2D<CustomComplex<dataType>> &aqsntemp, Array2D<CustomComplex<dataType>> &I_eps_array, Array1D<dataType> &vcoul, \
        Array1D<dataType> &achtemp_re, Array1D<dataType> &achtemp_im, dataType &elapsedKernelTimer);

inline void correntess(CustomComplex<dataType> result)
{
    dataType re_diff = result.get_real() - -264241151.997370;
    dataType im_diff = result.get_imag() - 1321205760.015211;

    if(re_diff < 0.00001 && im_diff < 0.01)
        printf("\n!!!! SUCCESS - !!!! Correctness test passed :-D :-D\n\n");
    else
        printf("\n!!!! FAILURE - Correctness test failed :-( :-(  \n");
}

void noflagOCC_solver(size_t number_bands, size_t ngpown, size_t ncouls, Array1D<int>& inv_igp_index, Array1D<int>& indinv, Array1D<dataType>& wx_array, Array2D<CustomComplex<dataType>>& wtilde_array, \
        Array2D<CustomComplex<dataType>>& aqsmtemp, Array2D<CustomComplex<dataType>>& aqsntemp, Array2D<CustomComplex<dataType>>& I_eps_array, Array1D<dataType>& vcoul, \
        Array1D<dataType>& achtemp_re, Array1D<dataType>& achtemp_im, dataType &elapsedKernelTimer)
{
    timeval startKernelTimer, endKernelTimer;
    //Vars to use for reduction
    dataType ach_re0 = 0.00, ach_re1 = 0.00, ach_re2 = 0.00, \
        ach_im0 = 0.00, ach_im1 = 0.00, ach_im2 = 0.00;
    gettimeofday(&startKernelTimer, NULL);

    for(int my_igp=0; my_igp<ngpown; ++my_igp)
    {
        for(int n1 = 0; n1<number_bands; ++n1)
        {
            int indigp = inv_igp_index(my_igp);
            int igp = indinv(indigp);
            dataType achtemp_re_loc[nend-nstart], achtemp_im_loc[nend-nstart];
            for(int iw = nstart; iw < nend; ++iw) {achtemp_re_loc[iw] = 0.00; achtemp_im_loc[iw] = 0.00;}
            CustomComplex<dataType> sch_store1 = CustomComplex_conj(aqsmtemp(n1,igp))*  aqsntemp(n1,igp) * 0.5 * vcoul(igp);

            for(int ig = 0; ig<ncouls; ++ig)
            {
                for(int iw = nstart; iw < nend; ++iw)
                {
                    CustomComplex<dataType> wdiff = wx_array(iw) - wtilde_array(my_igp,ig);
                    CustomComplex<dataType> delw = wtilde_array(my_igp, ig)* CustomComplex_conj(wdiff) * (1/CustomComplex_real((wdiff * CustomComplex_conj(wdiff))));
                    CustomComplex<dataType> sch_array = delw  * I_eps_array(my_igp,ig) * sch_store1;

                    achtemp_re_loc[iw] += CustomComplex_real(sch_array);
                    achtemp_im_loc[iw] += CustomComplex_imag(sch_array);
                }
            }
            ach_re0 += achtemp_re_loc[0];
            ach_re1 += achtemp_re_loc[1];
            ach_re2 += achtemp_re_loc[2];
            ach_im0 += achtemp_im_loc[0];
            ach_im1 += achtemp_im_loc[1];
            ach_im2 += achtemp_im_loc[2];
        } //ngpown
    } //number_bands

    gettimeofday(&endKernelTimer, NULL);
    elapsedKernelTimer = (endKernelTimer.tv_sec - startKernelTimer.tv_sec) +1e-6*(endKernelTimer.tv_usec - startKernelTimer.tv_usec);

    achtemp_re(0) = ach_re0;
    achtemp_re(1) = ach_re1;
    achtemp_re(2) = ach_re2;
    achtemp_im(0) = ach_im0;
    achtemp_im(1) = ach_im1;
    achtemp_im(2) = ach_im2;
}

int main(int argc, char** argv)
{

    int number_bands = 0, nvband = 0, ncouls = 0, nodes_per_group = 0;
    int npes = 1;
    if(argc == 1)
    {
        number_bands = 512;
        nvband = 2;
        ncouls = 32768;
        nodes_per_group = 20;
    }
    else if (argc == 5)
    {
        number_bands = atoi(argv[1]);
        nvband = atoi(argv[2]);
        ncouls = atoi(argv[3]);
        nodes_per_group = atoi(argv[4]);
    }
    else
    {
        std::cout << "The correct form of input is : " << endl;
        std::cout << " ./a.out <number_bands> <number_valence_bands> <number_plane_waves> <nodes_per_mpi_group> " << endl;
        exit (0);
    }
    int ngpown = ncouls / (nodes_per_group * npes);

//Constants that will be used later
    const dataType e_lk = 10;
    const dataType dw = 1;
    const dataType to1 = 1e-6;
    const dataType gamma = 0.5;
    const dataType sexcut = 4.0;
    const dataType limitone = 1.0/(to1*4.0);
    const dataType limittwo = pow(0.5,2);
    const dataType e_n1kq= 6.0;
    const dataType occ=1.0;

    //Start the timer before the work begins.
    dataType elapsedKernelTimer, elapsedTimer;
    timeval startTimer, endTimer;
    gettimeofday(&startTimer, NULL);
    timeval startKernelTimer, endKernelTimer;


    //OpenMP Printing of threads on Host and Device
    int tid, numThreads, numTeams;
    //Printing out the params passed.
    std::cout << "Sizeof(CustomComplex<dataType> = " << sizeof(CustomComplex<dataType>) << " bytes" << std::endl;
    std::cout << "number_bands = " << number_bands \
        << "\t nvband = " << nvband \
        << "\t ncouls = " << ncouls \
        << "\t nodes_per_group  = " << nodes_per_group \
        << "\t ngpown = " << ngpown \
        << "\t nend = " << nend \
        << "\t nstart = " << nstart << endl;

    CustomComplex<dataType> expr0(0.00, 0.00);
    CustomComplex<dataType> expr(0.5, 0.5);
    size_t memFootPrint = 0.00;

    //ALLOCATE statements from fortran gppkernel.
    Array1D<CustomComplex<dataType>> achtemp(nend-nstart);
    memFootPrint += achtemp.getSizeInBytes();

    Array2D<CustomComplex<dataType>> aqsmtemp(number_bands, ncouls);
    Array2D<CustomComplex<dataType>> aqsntemp(number_bands, ncouls);
    memFootPrint += 2*aqsmtemp.getSizeInBytes();

    Array2D<CustomComplex<dataType>> I_eps_array(ngpown, ncouls);
    Array2D<CustomComplex<dataType>> wtilde_array(ngpown, ncouls);
    memFootPrint += 2*I_eps_array.getSizeInBytes();

    Array1D<dataType> vcoul(ncouls);
    memFootPrint += vcoul.getSizeInBytes();;

    Array1D<int> inv_igp_index(ngpown);
    Array1D<int> indinv(ncouls+1);
    memFootPrint += inv_igp_index.getSizeInBytes();;
    memFootPrint += indinv.getSizeInBytes();;

//Real and imaginary parts of achtemp calculated separately to avoid critical.
    Array1D<dataType> achtemp_re(nend-nstart);
    Array1D<dataType> achtemp_im(nend-nstart);
    Array1D<dataType> wx_array(nend-nstart);
    memFootPrint += 3*wx_array.getSizeInBytes();

    //Print Memory Foot print
    cout << "Memory Foot Print = " << memFootPrint / pow(1024,3) << " GBs" << endl;


   for(int i=0; i<number_bands; i++)
       for(int j=0; j<ncouls; j++)
       {
           aqsmtemp(i,j) = expr;
           aqsntemp(i,j) = expr;
       }

   for(int i=0; i<ngpown; i++)
       for(int j=0; j<ncouls; j++)
       {
           I_eps_array(i,j) = expr;
           wtilde_array(i,j) = expr;
       }

   for(int i=0; i<ncouls; i++)
       vcoul(i) = 1.0;


    for(int ig=0; ig < ngpown; ++ig)
        inv_igp_index(ig) = (ig+1) * ncouls / ngpown;

    for(int ig=0; ig<ncouls; ++ig)
        indinv(ig) = ig;
        indinv(ncouls) = ncouls-1;

       for(int iw=nstart; iw<nend; ++iw)
       {
           achtemp_re(iw) = 0.00;
           achtemp_im(iw) = 0.00;
       }

        for(int iw=nstart; iw<nend; ++iw)
        {
            wx_array(iw) = e_lk - e_n1kq + dw*((iw+1)-2);
            if(wx_array(iw) < to1) wx_array(iw) = to1;
        }

        device_Array1D<double> d_achtemp_re(nend-nstart);
        device_Array1D<double> d_achtemp_im(nend-nstart);

        device_Array1D<int> d_inv_igp_index(ngpown);
        device_Array1D<int> d_indinv(ncouls+1);
        device_Array1D<dataType> d_wx_array(nend-nstart);
        device_Array1D<dataType> d_vcoul(ncouls);
        device_Array2D<CustomComplex<dataType>> d_aqsmtemp(number_bands, ncouls);
        device_Array2D<CustomComplex<dataType>> d_aqsntemp(number_bands, ncouls);
        device_Array2D<CustomComplex<dataType>> d_I_eps_array(ngpown, ncouls);
        device_Array2D<CustomComplex<dataType>> d_wtilde_array(ngpown, ncouls);

        copyToDevice(d_achtemp_re.dptr, achtemp_re.dptr, achtemp_re.size);
        copyToDevice(d_achtemp_im.dptr, achtemp_im.dptr, achtemp_im.size);
        copyToDevice(d_inv_igp_index.dptr, inv_igp_index.dptr, inv_igp_index.size);
        copyToDevice(d_indinv.dptr, indinv.dptr, indinv.size);
        copyToDevice(d_vcoul.dptr, vcoul.dptr, ncouls);
        copyToDevice(d_aqsmtemp.dptr, aqsmtemp.dptr, aqsmtemp.size);
        copyToDevice(d_aqsntemp.dptr, aqsntemp.dptr, aqsntemp.size);
        copyToDevice(d_I_eps_array.dptr, I_eps_array.dptr, I_eps_array.size);
        copyToDevice(d_wtilde_array.dptr, wtilde_array.dptr, wtilde_array.size);
        copyToDevice(d_wx_array.dptr, wx_array.dptr, wx_array.size);


//  checkCudaErrors(cudaGetDeviceProperties(&deviceProp, device_id));
//    noflagOCC_solver(number_bands, ngpown, ncouls, inv_igp_index, indinv, wx_array, wtilde_array, aqsmtemp, aqsntemp, I_eps_array, vcoul, achtemp_re, achtemp_im, elapsedKernelTimer);

    gettimeofday(&startKernelTimer, NULL);

    noflagOCC_cudaKernel(number_bands, ngpown, ncouls, d_achtemp_re, d_achtemp_im, \
            d_inv_igp_index, d_indinv, d_wx_array, \
            d_aqsmtemp, d_aqsntemp, d_vcoul, \
            d_wtilde_array, d_I_eps_array);

    cudaDeviceSynchronize();
    copyToHost(achtemp_re.dptr, d_achtemp_re.dptr, (nend-nstart));
    copyToHost(achtemp_im.dptr, d_achtemp_im.dptr, (nend-nstart));

    gettimeofday(&endKernelTimer, NULL);
    elapsedKernelTimer = (endKernelTimer.tv_sec - startKernelTimer.tv_sec) +1e-6*(endKernelTimer.tv_usec - startKernelTimer.tv_usec);


    for(int iw=nstart; iw<nend; ++iw)
        achtemp(iw) = CustomComplex<dataType>(achtemp_re(iw), achtemp_im(iw));

    //Check for correctness
    correntess(achtemp(0));
    printf("\n Final achtemp\n");
        achtemp(0).print();
    gettimeofday(&endTimer, NULL);
    elapsedTimer = (endTimer.tv_sec - startTimer.tv_sec) +1e-6*(endTimer.tv_usec - startTimer.tv_usec);

    cout << "********** Kernel Time Taken **********= " << elapsedKernelTimer << " secs" << endl;
    cout << "********** Total Time Taken **********= " << elapsedTimer << " secs" << endl;

    return 0;
}
