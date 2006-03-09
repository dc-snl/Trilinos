#ifndef MLAPI_EIG_H
#define MLAPI_EIG_H

/*!
\file MLAPI_Eig.h

\brief MLAPI interfaces to eigensolvers.

\author Marzio Sala, D-INFK/ETHZ.

\date Last updated on Mar-06.
*/

#include "ml_common.h"

namespace MLAPI {

class Operator;
class MultiVector;

//! Computes the maximum eigenvalue of \c Op using the A-norm of the operator.
double MaxEigAnorm(const Operator& Op, const bool DiagonalScaling = false);

//! Computes the maximum eigenvalue of \c Op using the CG method.
double MaxEigCG(const Operator& Op, const bool DiagonalScaling = false);

//! Computes the maximum eigenvalue of \c Op using the power method.
double MaxEigPowerMethod(const Operator& Op, const bool DiagonalScaling = false);

//! Computes the maximum eigenvalue of \c Op using Anasazi
double MaxEigAnasazi(const Operator& Op, const bool DiagonalScaling = false);

//! Computes eigenvalues and eigenvectors using LAPACK (w/ one process only).
void Eig(const Operator& Op, MultiVector& ER, MultiVector& EI);

void Eigs(const Operator& A, int NumEigenvalues, 
          MultiVector& ER, MultiVector& EI);

} // namespace MLAPI

#endif // MLAPI_EIG_H
