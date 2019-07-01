// Complex number computation class'
//
#ifndef __CustomComplex
#define __CustomComplex

#include "../arrayMD/arrayMDcpu.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>
#include <omp.h>
#include <stdio.h>
#include <sys/time.h>

#ifdef __CUDACC__
#include "../arrayMD/arrayMDgpu.h"
#endif

#define CUDA_HOSTDEV __host__ __device__

using namespace std;
using dataType = double;
#define nstart 0
#define nend 3

template <class type>

class CustomComplex {

private:
public:
  type x;
  type y;
  explicit CustomComplex() {
    x = 0.00;
    y = 0.00;
  }

#ifdef __CUDACC__
  CUDA_HOSTDEV
#endif
  explicit CustomComplex(const dataType &a, const dataType &b) {
    x = a;
    y = b;
  }

#ifdef __CUDACC__
  CUDA_HOSTDEV
#endif
  CustomComplex(const CustomComplex &src) {
    x = src.x;
    y = src.y;
  }

#ifdef __CUDACC__
  CUDA_HOSTDEV
#endif
  CustomComplex &operator=(const CustomComplex &src) {
    x = src.x;
    y = src.y;

    return *this;
  }

#ifdef __CUDACC__
  CUDA_HOSTDEV
#endif
  CustomComplex &operator+=(const CustomComplex &src) {
    x = src.x + this->x;
    y = src.y + this->y;

    return *this;
  }

#ifdef __CUDACC__
  CUDA_HOSTDEV
#endif
  CustomComplex &operator-=(const CustomComplex &src) {
    x = src.x - this->x;
    y = src.y - this->y;

    return *this;
  }

#ifdef __CUDACC__
  CUDA_HOSTDEV
#endif
  CustomComplex &operator-() {
    x = -this->x;
    y = -this->y;

    return *this;
  }

#ifdef __CUDACC__
  CUDA_HOSTDEV
#endif
  CustomComplex &operator~() { return *this; }

  void print() const {
    printf("( %f, %f) ", this->x, this->y);
    printf("\n");
  }

  dataType get_real() const { return this->x; }

  dataType get_imag() const { return this->y; }

  void set_real(dataType val) { this->x = val; }

  void set_imag(dataType val) { this->y = val; }

  // 6 flops
  template <class T>
#ifdef __CUDACC__
  CUDA_HOSTDEV
#endif
      friend inline CustomComplex<T>
      operator*(const CustomComplex<T> &a, const CustomComplex<T> &b) {
    T x_this = a.x * b.x - a.y * b.y;
    T y_this = a.x * b.y + a.y * b.x;
    CustomComplex<T> result(x_this, y_this);
    return (result);
  }

  // 2 flops
  template <class T>
#ifdef __CUDACC__
  CUDA_HOSTDEV
#endif
      friend inline CustomComplex<T>
      operator*(const CustomComplex<T> &a, const dataType &b) {
    CustomComplex<T> result(a.x * b, a.y * b);
    return result;
  }

  template <class T>
#ifdef __CUDACC__
  CUDA_HOSTDEV
#endif
      friend inline CustomComplex<T>
      operator*(const dataType &b, const CustomComplex<T> &a) {
    CustomComplex<T> result(a.x * b, a.y * b);
    return result;
  }

  template <class T>
#ifdef __CUDACC__
  CUDA_HOSTDEV
#endif
      friend inline CustomComplex<T>
      operator-(CustomComplex<T> a, CustomComplex<T> b) {
    CustomComplex<T> result(a.x - b.x, a.y - b.y);
    return result;
  }

  // 2 flops
  template <class T>
#ifdef __CUDACC__
  CUDA_HOSTDEV
#endif
      friend inline CustomComplex<T>
      operator-(const dataType &a, CustomComplex<T> &src) {
    CustomComplex<T> result(a - src.x, 0 - src.y);
    return result;
  }

  template <class T>
#ifdef __CUDACC__
  CUDA_HOSTDEV
#endif
      friend inline CustomComplex<T>
      operator+(const dataType &a, CustomComplex<T> &src) {
    CustomComplex<T> result(a + src.x, src.y);
    return result;
  }

  template <class T>
#ifdef __CUDACC__
  CUDA_HOSTDEV
#endif
      friend inline CustomComplex<T>
      operator+(CustomComplex<T> a, CustomComplex<T> b) {
    CustomComplex<T> result(a.x + b.x, a.y + b.y);
    return result;
  }

  template <class T>
#ifdef __CUDACC__
  CUDA_HOSTDEV
#endif
      friend inline CustomComplex<T>
      operator/(CustomComplex<T> a, CustomComplex<T> b) {

    CustomComplex<T> b_conj = CustomComplex_conj(b);
    CustomComplex<T> numerator = a * b_conj;
    CustomComplex<T> denominator = b * b_conj;

    dataType re_this = numerator.x / denominator.x;
    dataType im_this = numerator.y / denominator.x;

    CustomComplex<T> result(re_this, im_this);
    return result;
  }

  template <class T>
#ifdef __CUDACC__
  CUDA_HOSTDEV
#endif
      friend inline CustomComplex<T>
      operator/(CustomComplex<T> a, T b) {
    CustomComplex<T> result(a.x / b, a.y / b);
    return result;
  }

  template <class T>
#ifdef __CUDACC__
  CUDA_HOSTDEV
#endif
      friend inline CustomComplex<T>
      CustomComplex_conj(const CustomComplex<T> &src);

  template <class T>
#ifdef __CUDACC__
  CUDA_HOSTDEV
#endif
      friend inline dataType
      CustomComplex_abs(const CustomComplex<T> &src);

  template <class T>
#ifdef __CUDACC__
  CUDA_HOSTDEV
#endif
      friend inline dataType
      CustomComplex_real(const CustomComplex<T> &src);

  template <class T>
#ifdef __CUDACC__
  CUDA_HOSTDEV
#endif
      friend inline dataType
      CustomComplex_imag(const CustomComplex<T> &src);
};

/* Return the conjugate of a complex number
flop
*/
template <class T>
inline CustomComplex<T> CustomComplex_conj(const CustomComplex<T> &src) {

  T re_this = src.x;
  T im_this = -1 * src.y;

  CustomComplex<T> result(re_this, im_this);
  return result;
}

/*
 * Return the absolute of a complex number
 */
template <class T>
inline dataType CustomComplex_abs(const CustomComplex<T> &src) {
  T re_this = src.x * src.x;
  T im_this = src.y * src.y;

  T result = sqrt(re_this + im_this);
  return result;
}

/*
 * Return the real part of a complex number
 */
template <class T>
inline dataType CustomComplex_real(const CustomComplex<T> &src) {
  return src.x;
}

/*
 * Return the imaginary part of a complex number
 */
template <class T>
inline dataType CustomComplex_imag(const CustomComplex<T> &src) {
  return src.y;
}

#endif
