// @HEADER
// ************************************************************************
//
//                           Intrepid Package
//                 Copyright (2007) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions: Alejandro Mota (amota@sandia.gov)
//
// ************************************************************************
// @HEADER

#if !defined(Intrepid_MiniTensor_TensorBase_i_h)
#define Intrepid_MiniTensor_TensorBase_i_h

namespace Intrepid
{

//
// Default constructor.
//
template<typename T, typename Store>
inline
TensorBase<T, Store>::TensorBase() :
dimension_(0)
{
  if (Store::IS_DYNAMIC == true) {
    set_number_components(0);
  } else {
    fill(NANS);
  }
  return;
}

//
// Construction that initializes to NaNs
//
template<typename T, typename Store>
inline
TensorBase<T, Store>::TensorBase(Index const dimension, Index const order) :
dimension_(0)
{
  set_dimension(dimension, order);

  fill(NANS);

  return;
}

//
// Create with specified value
//
template<typename T, typename Store>
inline
TensorBase<T, Store>::TensorBase(
    Index const dimension,
    Index const order,
    ComponentValue const value) :
    dimension_(0)
{
  set_dimension(dimension, order);

  fill(value);

  return;
}

//
// Construction from a scalar
//
template<typename T, typename Store>
inline
TensorBase<T, Store>::TensorBase(
    Index const dimension,
    Index const order,
    T const & s) :
    dimension_(0)
{
  set_dimension(dimension, order);

  fill(s);

  return;
}

//
// Construction from array
//
template<typename T, typename Store>
inline
TensorBase<T, Store>::TensorBase(
    Index const dimension,
    Index const order,
    T const * data_ptr) :
    dimension_(0)
{
  set_dimension(dimension, order);

  fill(data_ptr);

  return;
}

//
// Copy constructor
//
template<typename T, typename Store>
inline
TensorBase<T, Store>::TensorBase(TensorBase<T, Store> const & X) :
dimension_(X.dimension_)
{
  Index const
  number_components = X.get_number_components();

  set_number_components(number_components);

  for (Index i = 0; i < number_components; ++i) {
    (*this)[i] = X[i];
  }

  return;
}

//
// Copy assignment
//
template<typename T, typename Store>
inline
TensorBase<T, Store> &
TensorBase<T, Store>::operator=(TensorBase<T, Store> const & X)
{
  if (this == &X) return *this;

  dimension_ = X.dimension_;

  Index const
  number_components = X.get_number_components();

  set_number_components(number_components);

  for (Index i = 0; i < number_components; ++i) {
    (*this)[i] = X[i];
  }

  return *this;
}

//
// Simple destructor
//
template<typename T, typename Store>
inline
TensorBase<T, Store>::~TensorBase()
{
  return;
}

//
// Get dimension
//
template<typename T, typename Store>
inline
Index
TensorBase<T, Store>::get_dimension() const
{
  assert(Store::IS_DYNAMIC == true);

  return dimension_;
}

//
// Set dimension
//
template<typename T, typename Store>
inline
void
TensorBase<T, Store>::set_dimension(Index const dimension, Index const order)
{
  if (Store::IS_STATIC == true) return;

  dimension_ = dimension;

  Index const
  number_components = integer_power(dimension, order);

  set_number_components(number_components);

  return;
}

//
// Linear access to components
//
template<typename T, typename Store>
inline
T const &
TensorBase<T, Store>::operator[](Index const i) const
{
  return components_[i];
}

//
// Linear access to components
//
template<typename T, typename Store>
inline
T &
TensorBase<T, Store>::operator[](Index const i)
{
  return components_[i];
}

//
// Get total number of components
//
template<typename T, typename Store>
inline
Index
TensorBase<T, Store>::get_number_components() const
{
  return components_.size();
}

//
// Allocate space for components
//
template<typename T, typename Store>
inline
void
TensorBase<T, Store>::set_number_components(Index const number_components)
{
  components_.resize(number_components);

  return;
}

//
// Fill components with value.
//
template<typename T, typename Store>
inline
void
TensorBase<T, Store>::fill(ComponentValue const value)
{
  Index const
  number_components = get_number_components();

  switch (value) {

    case ZEROS:
      for (Index i = 0; i < number_components; ++i) {
        (*this)[i] = 0;
      }
      break;

    case ONES:
      for (Index i = 0; i < number_components; ++i) {
        (*this)[i] = 1;
      }
      break;

    case SEQUENCE:
      for (Index i = 0; i < number_components; ++i) {
        (*this)[i] = i;
      }
      break;

    case RANDOM:
      for (Index i = 0; i < number_components; ++i) {
        (*this)[i] = random<T>();
      }
      break;

    case NANS:
      for (Index i = 0; i < number_components; ++i) {
        (*this)[i] = not_a_number<T>();
      }
      break;

    default:
      std::cerr << "ERROR: " << __PRETTY_FUNCTION__;
      std::cerr << std::endl;
      std::cerr << "Unknown specification of value for filling components.";
      std::cerr << std::endl;
      exit(1);
      break;
  }

  return;
}

//
// Fill components from argument
//
template<typename T, typename Store>
inline
void
TensorBase<T, Store>::fill(T const & s)
{
  Index const
  number_components = get_number_components();

  for (Index i = 0; i < number_components; ++i) {
    (*this)[i] = s;
  }

  return;
}

//
// Fill components from array defined by pointer.
//
template<typename T, typename Store>
inline
void
TensorBase<T, Store>::fill(T const * data_ptr)
{
  assert(data_ptr != NULL);

  Index const
  number_components = get_number_components();

  for (Index i = 0; i < number_components; ++i) {
    (*this)[i] = data_ptr[i];
  }

  return;
}

//
// Component increment
//
template<typename T, typename Store>
inline
TensorBase<T, Store> &
TensorBase<T, Store>::operator+=(TensorBase<T, Store> const & X)
{
  Index const
  number_components = get_number_components();

  assert(number_components == X.get_number_components());

  for (Index i = 0; i < number_components; ++i) {
    (*this)[i] += X[i];
  }

  return *this;
}

//
// Component decrement
//
template<typename T, typename Store>
inline
TensorBase<T, Store> &
TensorBase<T, Store>::operator-=(TensorBase<T, Store> const & X)
{
  Index const
  number_components = get_number_components();

  assert(number_components == X.get_number_components());

  for (Index i = 0; i < number_components; ++i) {
    (*this)[i] -= X[i];
  }

  return *this;
}

//
// Fill with zeros
//
template<typename T, typename Store>
inline
void
TensorBase<T, Store>::clear()
{
  fill(ZEROS);
  return;
}

//
// Square of Frobenius norm
//
template<typename T, typename Store>
T
norm_f_square(TensorBase<T, Store> const & X)
{
  T
  s = 0.0;

  for (Index i = 0; i < X.get_number_components(); ++i) {
    s += X[i] * X[i];
  }

  return s;
}

//
// Frobenius norm
//
template<typename T, typename Store>
T
norm_f(TensorBase<T, Store> const & X)
{
  return std::sqrt(norm_f_square(X));
}

//
// Base addition
//
template<typename R, typename S, typename T, typename Store>
void
add(
    TensorBase<R, Store> const & A,
    TensorBase<S, Store> const & B,
    TensorBase<T, Store> & C
)
{
  Index const
  number_components = A.get_number_components();

  assert(B.get_number_components() == number_components);
  assert(C.get_number_components() == number_components);

  for (Index i = 0; i < number_components; ++i) {
    C[i] = A[i] + B[i];
  }

  return;
}

//
// Base subtraction
//
template<typename R, typename S, typename T, typename Store>
void
subtract(
    TensorBase<R, Store> const & A,
    TensorBase<S, Store> const & B,
    TensorBase<T, Store> & C)
{
  Index const
  number_components = A.get_number_components();

  assert(B.get_number_components() == number_components);
  assert(C.get_number_components() == number_components);

  for (Index i = 0; i < number_components; ++i) {
    C[i] = A[i] - B[i];
  }

  return;
}

//
// Base minus
//
template<typename T, typename Store>
void
minus(TensorBase<T, Store> const & A, TensorBase<T, Store> & B)
{
  Index const
  number_components = A.get_number_components();

  assert(B.get_number_components() == number_components);

  for (Index i = 0; i < number_components; ++i) {
    B[i] = - A[i];
  }

  return;
}

//
// Base equality
//
template<typename T, typename Store>
bool
equal(TensorBase<T, Store> const & A, TensorBase<T, Store> const & B)
{
  Index const
  number_components = A.get_number_components();

  assert(B.get_number_components() == number_components);

  for (Index i = 0; i < number_components; ++i) {
    if (A[i] != B[i]) return false;
  }

  return true;
}

//
// Base not equality
//
template<typename T, typename Store>
bool
not_equal(TensorBase<T, Store> const & A, TensorBase<T, Store> const & B)
{
  return !(equal(A, B));
}

//
// Base scaling
//
template<typename R, typename S, typename T, typename Store>
void
scale(TensorBase<R, Store> const & A, S const & s, TensorBase<T, Store> & B)
{
  Index const
  number_components = A.get_number_components();

  assert(B.get_number_components() == number_components);

  for (Index i = 0; i < number_components; ++i) {
    B[i] = s * A[i];
  }

  return;
}

//
// Base division
//
template<typename R, typename S, typename T, typename Store>
void
divide(TensorBase<R, Store> const & A, S const & s, TensorBase<T, Store> & B)
{
  Index const
  number_components = A.get_number_components();

  assert(B.get_number_components() == number_components);

  for (Index i = 0; i < number_components; ++i) {
    B[i] = A[i] / s;
  }

  return;
}

//
// Base split (scalar divided by tensor)
//
template<typename R, typename S, typename T, typename Store>
void
split(TensorBase<R, Store> const & A, S const & s, TensorBase<T, Store> & B)
{
  Index const
  number_components = A.get_number_components();

  assert(B.get_number_components() == number_components);

  for (Index i = 0; i < number_components; ++i) {
    B[i] = s / A[i];
  }

  return;
}

} // namespace Intrepid

#endif // Intrepid_MiniTensor_TensorBase_i_h
