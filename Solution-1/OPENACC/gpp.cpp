#include "../../external/ComplexClass/CustomComplex.h"
#include "../../external/arrayMD/arrayMDcpu.h"
#include <string.h>

#define nstart 0
#define nend 3

using dataType=double;

void noflagOCC_solver(size_t number_bands, size_t ngpown, size_t ncouls, Array1D<int> &inv_igp_index, Array1D<int> &indinv, Array1D<dataType>& wx_array, Array2D<CustomComplex<dataType>>& wtilde_array, \
        Array2D<CustomComplex<dataType>> &aqsmtemp, Array2D<CustomComplex<dataType>> &aqsntemp, Array2D<CustomComplex<dataType>> &I_eps_array, Array1D<dataType> &vcoul, \
        Array1D<dataType> &achtemp_re, Array1D<dataType> &achtemp_im, dataType &elapsedKernelTimer);

inline void correntess(int problem_size, CustomComplex<dataType> result)
{
    if(problem_size == 0)
    {
        dataType re_diff = result.get_real() - -24852.551551;
        dataType im_diff = result.get_imag() - 2957453.636523;

        if(re_diff < 0.00001 && im_diff < 0.00001)
            printf("\n!!!! SUCCESS - !!!! Correctness test passed :-D :-D\n\n");
        else
            printf("\n!!!! FAILURE - Correctness test failed :-( :-(  \n");
    }
    else
    {
        dataType re_diff = result.get_real() - -0.096066;
        dataType im_diff = result.get_imag() - 11.431852;

        if(re_diff < 0.00001 && im_diff < 0.00001)
            printf("\n!!!! SUCCESS - !!!! Correctness test passed :-D :-D\n\n");
        else
            printf("\n!!!! FAILURE - Correctness test failed :-( :-(  \n");
    }
}

void noflagOCC_solver(size_t number_bands, size_t ngpown, size_t ncouls, Array1D<int> &inv_igp_index, Array1D<int> &indinv, Array1D<dataType>& wx_array, Array2D<CustomComplex<dataType>>& wtilde_array, \
        Array2D<CustomComplex<dataType>> &aqsmtemp, Array2D<CustomComplex<dataType>> &aqsntemp, Array2D<CustomComplex<dataType>> &I_eps_array, Array1D<dataType> &vcoul, \
        Array1D<dataType> &achtemp_re, Array1D<dataType> &achtemp_im, dataType &elapsedKernelTimer)
{
    timeval startKernelTimer, endKernelTimer;
    gettimeofday(&startKernelTimer, NULL);

    dataType ach_re0 = 0.00, ach_re1 = 0.00, ach_re2 = 0.00, \
        ach_im0 = 0.00, ach_im1 = 0.00, ach_im2 = 0.00;

#pragma acc parallel loop gang vector \
    copyin(wx_array[0:1], wtilde_array[0:1], wx_array.dptr[0:wx_array.size], wtilde_array.dptr[0:wtilde_array.size], \
            I_eps_array[0:1], I_eps_array.dptr[0:I_eps_array.size], aqsmtemp[0:1], aqsmtemp.dptr[0:aqsmtemp.size], aqsntemp[0:1], aqsntemp.dptr[0:aqsntemp.size], \
            vcoul[0:1], vcoul.dptr[0:vcoul.size], \
            inv_igp_index[0:1], inv_igp_index.dptr[0:inv_igp_index.size], indinv[0:1], indinv.dptr[0:indinv.size]) \
    copy(achtemp_re, achtemp_im, achtemp_re.dptr[0:achtemp_re.size], achtemp_im.dptr[0:achtemp_im.size])\
    reduction(+:ach_re0, ach_re1, ach_re2, ach_im0, ach_im1, ach_im2)
    for(int n1 = 0; n1<number_bands; ++n1) //512
    {
        for(int my_igp=0; my_igp<ngpown; ++my_igp) //1634
        {
            int indigp = inv_igp_index(my_igp);
            int igp = indinv(indigp);
            dataType achtemp_re_loc[nend-nstart], achtemp_im_loc[nend-nstart];
            for(int iw = nstart; iw < nend; ++iw) {achtemp_re_loc[iw] = 0.00; achtemp_im_loc[iw] = 0.00;}

            for(int ig = 0; ig<ncouls; ++ig) //32768
            {
                for(int iw = nstart; iw < nend; ++iw) //3
                {
                    CustomComplex<dataType> wdiff = wx_array(iw) - wtilde_array(my_igp,ig);
                    CustomComplex<dataType> delw = wtilde_array(my_igp, ig)* CustomComplex_conj(wdiff) * (1/CustomComplex_real((wdiff * CustomComplex_conj(wdiff))));
                    CustomComplex<dataType> sch_array = delw  * I_eps_array(my_igp,ig) * CustomComplex_conj(aqsmtemp(n1,igp))*  aqsntemp(n1,igp) * 0.5 * vcoul(igp);

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

    achtemp_re(0) = ach_re0;
    achtemp_re(1) = ach_re1;
    achtemp_re(2) = ach_re2;
    achtemp_im(0) = ach_im0;
    achtemp_im(1) = ach_im1;
    achtemp_im(2) = ach_im2;

    gettimeofday(&endKernelTimer, NULL);
    elapsedKernelTimer = (endKernelTimer.tv_sec - startKernelTimer.tv_sec) +1e-6*(endKernelTimer.tv_usec - startKernelTimer.tv_usec);
}

int main(int argc, char** argv)
{

    cout << "\n ************OPENACC VERSION 1  **********\n" << endl;

    int number_bands = 0, nvband = 0, ncouls = 0, nodes_per_group = 0;
    if(argc == 1)
    {
        number_bands = 512;
        nvband = 2;
        ncouls = 32768;
        nodes_per_group = 20;
    }
    else if(argc == 2)
    {
        if(strcmp(argv[1], "benchmark_problem") == 0)
        {
            number_bands = 512;
            nvband = 2;
            ncouls = 32768;
            nodes_per_group = 20;
        }
        else if(strcmp(argv[1], "test_problem") == 0)
        {
            number_bands = 512;
            nvband = 2;
            ncouls = 512;
            nodes_per_group = 20;
        }
        else
            std::cout << "The problem sizes are either benchmark_problem or test_problem" << std::endl;
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
    int ngpown = ncouls / nodes_per_group;

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
    CustomComplex<dataType> expr(0.025, 0.025);
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
       vcoul(i) = i*0.025;


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

    //The solver kernel
    noflagOCC_solver(number_bands, ngpown, ncouls, inv_igp_index, indinv, wx_array, wtilde_array, aqsmtemp, aqsntemp, I_eps_array, vcoul, achtemp_re, achtemp_im, elapsedKernelTimer);

    for(int iw=nstart; iw<nend; ++iw)
        achtemp(iw) = CustomComplex<dataType>(achtemp_re(iw), achtemp_im(iw));

    //Check for correctness
    if(strcmp(argv[1], "benchmark_problem") == 0)
        correntess(0,achtemp(0));
    else
        correntess(1,achtemp(0));

    printf("\n Final achtemp\n");
        achtemp(0).print();

    gettimeofday(&endTimer, NULL);
    elapsedTimer = (endTimer.tv_sec - startTimer.tv_sec) +1e-6*(endTimer.tv_usec - startTimer.tv_usec);

    cout << "********** Kernel Time Taken **********= " << elapsedKernelTimer << " secs" << endl;
    cout << "********** Total Time Taken **********= " << elapsedTimer << " secs" << endl;

    return 0;
}