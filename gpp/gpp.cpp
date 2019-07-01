#include <string.h>

#include "CustomComplex.h"
#include "commonDefines.h"

#define nstart 0
#define nend 3

using dataType = double;

inline void *safe_malloc(size_t n) {
  void *p = malloc(n);
  if (p == NULL) {
    fprintf(stderr, "Fatal: failed to allocate %zu bytes.\n", n);
    abort();
  }
  return p;
}

void noflagOCC_solver(size_t number_bands, size_t ngpown, size_t ncouls,
                      int *inv_igp_index, int *indinv, dataType *wx_array,
                      CustomComplex<dataType> *wtilde_array,
                      CustomComplex<dataType> *aqsmtemp,
                      CustomComplex<dataType> *aqsntemp,
                      CustomComplex<dataType> *I_eps_array, dataType *vcoul,
                      dataType *achtemp_re, dataType *achtemp_im,
                      dataType &elapsedKernelTimer);

// Here we are checking to see if the answers are correct
inline void correntess(int problem_size, CustomComplex<dataType> result) {
  if (problem_size == 0) {
    dataType re_diff = result.get_real() - -24852.551547;
    dataType im_diff = result.get_imag() - 2957453.638101;

    if (re_diff < 0.00001 && im_diff < 0.00001)
      printf("\nBenchmark Problem !!!! SUCCESS - !!!! Correctness test passed "
             ":-D :-D\n\n");
    else
      printf("\nBenchmark Problem !!!! FAILURE - Correctness test failed :-( "
             ":-(  \n");
  } else {
    dataType re_diff = result.get_real() - -0.096066;
    dataType im_diff = result.get_imag() - 11.431852;

    if (re_diff < 0.00001 && im_diff < 0.00001)
      printf("\nTest Problem !!!! SUCCESS - !!!! Correctness test passed :-D "
             ":-D\n\n");
    else
      printf(
          "\nTest Problem !!!! FAILURE - Correctness test failed :-( :-(  \n");
  }
}

void noflagOCC_solver(size_t number_bands, size_t ngpown, size_t ncouls,
                      int *inv_igp_index, int *indinv, dataType *wx_array,
                      CustomComplex<dataType> *wtilde_array,
                      CustomComplex<dataType> *aqsmtemp,
                      CustomComplex<dataType> *aqsntemp,
                      CustomComplex<dataType> *I_eps_array, dataType *vcoul,
                      dataType *achtemp_re, dataType *achtemp_im,
                      dataType &elapsedKernelTimer) {
  timeval startKernelTimer, endKernelTimer;
  gettimeofday(&startKernelTimer, NULL);

  //***********************  THIS IS THE MAIN LOOP ***************************
  // Focus your optimization efforts here.
  // You shouldn't need to change code anywhere else

  // hint: parallel pragma
  // hint: where are the data? target or device?
  // hint: data reduction

  for (size_t n1 = 0; n1 < number_bands; ++n1) // 512 iterations
  {

    // hint: think about loop ordering/loop collapsing
    for (size_t my_igp = 0; my_igp < ngpown; ++my_igp) // 1634 iterations
    {
      int indigp = inv_igp_index[my_igp];
      int igp = indinv[indigp];

      // 32768 iterations - most of the compute effort is here!
      for (size_t ig = 0; ig < ncouls; ++ig)
      {

        for (size_t iw = nstart; iw < nend; ++iw) // 3 iterations
        {
          CustomComplex<dataType> wdiff =
              wx_array[iw] - wtilde_array(my_igp, ig);
          CustomComplex<dataType> delw =
              wtilde_array(my_igp, ig) * CustomComplex_conj(wdiff) *
              (1 / CustomComplex_real((wdiff * CustomComplex_conj(wdiff))));
          CustomComplex<dataType> sch_array =
              delw * I_eps_array(my_igp, ig) *
              CustomComplex_conj(aqsmtemp(n1, igp)) * aqsntemp(n1, igp) * 0.5 *
              vcoul[igp];

          // hint: atomic operation
          achtemp_re[iw] += CustomComplex_real(sch_array);
          achtemp_im[iw] += CustomComplex_imag(sch_array);
        }
      }
    } // ngpown
  }   // number_bands
  //************************** END OF MAIN LOOP  *****************************

  gettimeofday(&endKernelTimer, NULL);
  elapsedKernelTimer =
      (endKernelTimer.tv_sec - startKernelTimer.tv_sec) +
      1e-6 * (endKernelTimer.tv_usec - startKernelTimer.tv_usec);
}

int main(int argc, char **argv) {

  cout << "\n ************SEQUENTIAL VERSION  **********\n" << endl;

  int number_bands = 0, nvband = 0, ncouls = 0, nodes_per_group = 0;
  if (argc == 1) {
    number_bands = 512;
    nvband = 2;
    ncouls = 512;
    nodes_per_group = 20;
  } else if (argc == 2) {
    if (strcmp(argv[1], "benchmark_problem") == 0) {
      number_bands = 512;
      nvband = 2;
      ncouls = 32768;
      nodes_per_group = 20;
    } else if (strcmp(argv[1], "test_problem") == 0) {
      number_bands = 512;
      nvband = 2;
      ncouls = 512;
      nodes_per_group = 20;
    } else {
      std::cout
          << "The problem sizes are either benchmark_problem or test_problem"
          << std::endl;
      exit(0);
    }
  } else if (argc == 5) {
    number_bands = atoi(argv[1]);
    nvband = atoi(argv[2]);
    ncouls = atoi(argv[3]);
    nodes_per_group = atoi(argv[4]);
  } else {
    std::cout << "The correct form of input is : " << endl;
    std::cout << " ./a.out <number_bands> <number_valence_bands> "
                 "<number_plane_waves> <nodes_per_mpi_group> "
              << endl;
    exit(0);
  }
  int ngpown = ncouls / nodes_per_group;

  // Constants that will be used later
  const dataType e_lk = 10;
  const dataType dw = 1;
  const dataType to1 = 1e-6;
  const dataType e_n1kq = 6.0;

  // Start the timer before the work begins.
  dataType elapsedKernelTimer, elapsedTimer;
  timeval startTimer, endTimer;
  gettimeofday(&startTimer, NULL);

  // Printing out the params passed.
  std::cout << "Sizeof(CustomComplex<dataType> = "
            << sizeof(CustomComplex<dataType>) << " bytes" << std::endl;
  std::cout << "number_bands = " << number_bands << "\t nvband = " << nvband
            << "\t ncouls = " << ncouls
            << "\t nodes_per_group  = " << nodes_per_group
            << "\t ngpown = " << ngpown << "\t nend = " << nend
            << "\t nstart = " << nstart << endl;

  CustomComplex<dataType> expr0(0.00, 0.00);
  CustomComplex<dataType> expr(0.025, 0.025);
  size_t memFootPrint = 0.00;

  // ALLOCATE statements from fortran gppkernel.
  CustomComplex<dataType> *achtemp;
  achtemp = (CustomComplex<dataType> *)safe_malloc(
      achtemp_size * sizeof(CustomComplex<dataType>));
  memFootPrint += achtemp_size * sizeof(CustomComplex<dataType>);

  CustomComplex<dataType> *aqsmtemp, *aqsntemp;
  aqsmtemp = (CustomComplex<dataType> *)safe_malloc(
      aqsmtemp_size * sizeof(CustomComplex<dataType>));
  aqsntemp = (CustomComplex<dataType> *)safe_malloc(
      aqsntemp_size * sizeof(CustomComplex<dataType>));
  memFootPrint += 2 * aqsmtemp_size * sizeof(CustomComplex<dataType>);

  CustomComplex<dataType> *I_eps_array, *wtilde_array;
  I_eps_array = (CustomComplex<dataType> *)safe_malloc(
      I_eps_array_size * sizeof(CustomComplex<dataType>));
  wtilde_array = (CustomComplex<dataType> *)safe_malloc(
      I_eps_array_size * sizeof(CustomComplex<dataType>));
  memFootPrint += 2 * I_eps_array_size * sizeof(CustomComplex<dataType>);

  dataType *vcoul;
  vcoul = (dataType *)safe_malloc(vcoul_size * sizeof(dataType));
  memFootPrint += vcoul_size * sizeof(dataType);

  int *inv_igp_index, *indinv;
  inv_igp_index = (int *)safe_malloc(inv_igp_index_size * sizeof(int));
  indinv = (int *)safe_malloc(indinv_size * sizeof(int));

  // Real and imaginary parts of achtemp calculated separately to avoid
  // critical.
  dataType *achtemp_re, *achtemp_im, *wx_array;
  achtemp_re = (dataType *)safe_malloc(achtemp_re_size * sizeof(dataType));
  achtemp_im = (dataType *)safe_malloc(achtemp_im_size * sizeof(dataType));
  wx_array = (dataType *)safe_malloc(wx_array_size * sizeof(dataType));
  memFootPrint += 3 * wx_array_size * sizeof(double);

  // Print Memory Foot print
  cout << "Memory Foot Print = " << memFootPrint / pow(1024, 3) << " GBs"
       << endl;

  for (int n1 = 0; n1 < number_bands; n1++)
    for (int ig = 0; ig < ncouls; ig++) {
      aqsmtemp(n1, ig) = expr;
      aqsntemp(n1, ig) = expr;
    }

  for (int my_igp = 0; my_igp < ngpown; my_igp++)
    for (int ig = 0; ig < ncouls; ig++) {
      I_eps_array(my_igp, ig) = expr;
      wtilde_array(my_igp, ig) = expr;
    }

  for (int i = 0; i < ncouls; i++)
    vcoul[i] = i * 0.025;

  for (int ig = 0; ig < ngpown; ++ig)
    inv_igp_index[ig] = (ig + 1) * ncouls / ngpown;

  for (int ig = 0; ig < ncouls; ++ig)
    indinv[ig] = ig;
  indinv[ncouls] = ncouls - 1;

  for (int iw = nstart; iw < nend; ++iw) {
    achtemp_re[iw] = 0.00;
    achtemp_im[iw] = 0.00;
  }

  for (int iw = nstart; iw < nend; ++iw) {
    wx_array[iw] = e_lk - e_n1kq + dw * ((iw + 1) - 2);
    if (wx_array[iw] < to1)
      wx_array[iw] = to1;
  }

  // The solver kernel -- this calls our MAIN LOOP (where you should focus your
  // optimizations!)
  noflagOCC_solver(number_bands, ngpown, ncouls, inv_igp_index, indinv,
                   wx_array, wtilde_array, aqsmtemp, aqsntemp, I_eps_array,
                   vcoul, achtemp_re, achtemp_im, elapsedKernelTimer);

  for (int iw = nstart; iw < nend; ++iw)
    achtemp[iw] = CustomComplex<dataType>(achtemp_re[iw], achtemp_im[iw]);

  // Check for correctness
  if (argc == 2) {
    if (strcmp(argv[1], "benchmark_problem") == 0)
      correntess(0, achtemp[0]);
    else if (strcmp(argv[1], "test_problem") == 0)
      correntess(1, achtemp[0]);
  } else
    correntess(1, achtemp[0]);

  printf("\n Final achtemp\n");
  achtemp[0].print();

  gettimeofday(&endTimer, NULL);
  elapsedTimer = (endTimer.tv_sec - startTimer.tv_sec) +
                 1e-6 * (endTimer.tv_usec - startTimer.tv_usec);

  // Free the allocated memory
  free(achtemp);
  free(aqsmtemp);
  free(aqsntemp);
  free(I_eps_array);
  free(wtilde_array);
  free(vcoul);
  free(inv_igp_index);
  free(indinv);
  free(achtemp_re);
  free(achtemp_im);
  free(wx_array);

  cout << "********** Kernel Time Taken **********= " << elapsedKernelTimer
       << " secs" << endl;
  cout << "********** Total Time Taken **********= " << elapsedTimer << " secs"
       << endl;

  return 0;
}
