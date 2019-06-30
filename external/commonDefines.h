#include <strings.h>
#include <stdio.h>
#include <stdlib.h>


#define aqsmtemp_size number_bands*ncouls
#define aqsntemp_size number_bands*ncouls
#define I_eps_array_size ngpown*ncouls
#define achtemp_size (nend-nstart)
#define achtemp_re_size (nend-nstart)
#define achtemp_im_size (nend-nstart)
#define vcoul_size ncouls
#define inv_igp_index_size ngpown
#define indinv_size (ncouls+1)
#define wx_array_size (nend-nstart)
#define wtilde_array_size ngpown*ncouls

#define aqsmtemp(n1,ig) aqsmtemp[n1*ncouls+ig]
#define aqsntemp(n1,ig) aqsntemp[n1*ncouls+ig]
#define I_eps_array(my_igp,ig) I_eps_array[my_igp*ncouls +ig]
#define wtilde_array(my_igp,ig) wtilde_array[my_igp*ncouls +ig]
