#@HEADER
# ***********************************************************************
#
#                           Rythmos Package
#                 Copyright (2006) Sandia Corporation
#
# Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
# license for use of this work by or on behalf of the U.S. Government.
#
# This library is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2.1 of the
# License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
# USA
# Questions? Contact Todd S. Coffey (tscoffe@sandia.gov)
#
# ***********************************************************************
#@HEADER

# The following line helps the test harness recover from build errors.

all-local:

#np# Set "SUBDIRS= (list all subdirectories that are autotool'ed here)"
#np# List all subdirectories even if one or more are configured/built only
#np# conditionally.
#np# For packages whose tests are located in this directory, rather than
#np# in a subdirectory of this directory, use the Makefile.am located in
#np# test1/ rather than this file.

if HAVE_RYTHMOS_EXPERIMENTAL
RYTHMOS_EXPERIMENTAL_TESTS = ThetaStepper
endif # HAVE_RYTHMOS_EXPERIMENTAL

SUBDIRS = $(RYTHMOS_EXPERIMENTAL_TESTS) \
					ExplicitTaylorPolynomialStepper\
          complicatedExample \
          UnitTest \
					ConvergenceTest \
          simpleAdjoint 

EXTRA_DIST = definition

