/**
 * @file make_alias.hpp
 * @author Ryan Curtin
 *
 * Implementation of `MakeAlias()`, a utility function.  This is meant to be
 * used in `SetWeights()` calls in various layers, to wrap internal weight
 * objects as aliases around the given memory pointers.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#ifndef MLPACK_METHODS_ANN_MAKE_ALIAS_HPP
#define MLPACK_METHODS_ANN_MAKE_ALIAS_HPP

#include <mlpack/prereqs.hpp>
#include <bandicoot>

namespace mlpack {

template<typename eT>
struct IsCootType;

template<typename eT>
struct IsCootType
{
  constexpr static bool value = false;
};

#ifdef MLPACK_HAS_COOT

template<typename eT>
struct IsCootType<coot::Mat<eT>>
{
  constexpr static bool value = true;
};

template<typename eT>
struct IsCootType<coot::subview_col<eT>>
{
  constexpr static bool value = true;
};

template<typename eT>
struct IsCootType<coot::Cube<eT>>
{
  constexpr static bool value = true;
};

#endif

/**
 * Reconstruct `m` as an alias around the memory `newMem`, with size `numRows` x
 * `numCols`.
 */
template<typename InMatType,
         typename OutMatType,
         std::enable_if_t<!IsCootType<InMatType>::value, bool> = true>
void MakeAlias(OutMatType& m,
               const InMatType& oldMat,
               const size_t numRows,
               const size_t numCols)
{
  // We use placement new to reinitialize the object, since the copy and move
  // assignment operators in Armadillo will end up copying memory instead of
  // making an alias.
  typename InMatType::elem_type* newMem = oldMat.memptr();
  m.~Mat();
  new (&m) OutMatType(newMem, numRows, numCols, false, true);
}

/**
 * Reconstruct `c` as an alias around the memory` newMem`, with size `numRows` x
 * `numCols` x `numSlices`.
 */
template<typename InCubeType,
         typename OutCubeType,
         std::enable_if_t<!IsCootType<InCubeType>::value, bool> = true>
void MakeAlias(OutCubeType& c,
               const InCubeType& oldCube,
               const size_t numRows,
               const size_t numCols,
               const size_t numSlices)
{
  // We use placement new to reinitialize the object, since the copy and move
  // assignment operators in Armadillo will end up copying memory instead of
  // making an alias.
  typename InCubeType::elem_type* newMem = oldCube.memptr();
  c.~Cube();
  new (&c) OutCubeType(newMem, numRows, numCols, numSlices, false, true);
}

template<typename InMatType,
         typename OutMatType,
         std::enable_if_t<IsCootType<InMatType>::value, bool> = true>
void MakeAlias(OutMatType& m,
               const InMatType& oldMat,
               const size_t numRows,
               const size_t numCols)
{
  // We use placement new to reinitialize the object, since the copy and move
  // assignment operators in Armadillo will end up copying memory instead of
  // making an alias.
  typename InMatType::elem_type* newMem = oldMat.get_dev_mem();
  m.~Mat();
  new (&m) OutMatType(newMem, numRows, numCols, false, true);
}

/**
 * Reconstruct `c` as an alias around the memory` newMem`, with size `numRows` x
 * `numCols` x `numSlices`.
 */
template<typename InCubeType,
         typename OutCubeType,
         std::enable_if_t<IsCootType<InCubeType>::value, bool> = true>
void MakeAlias(OutCubeType& c,
               const InCubeType& oldCube,
               const size_t numRows,
               const size_t numCols,
               const size_t numSlices)
{
  // We use placement new to reinitialize the object, since the copy and move
  // assignment operators in Armadillo will end up copying memory instead of
  // making an alias.
  typename InCubeType::elem_type* newMem = oldCube.get_dev_mem();
  c.~Cube();
  new (&c) OutCubeType(newMem, numRows, numCols, numSlices, false, true);
}

} // namespace mlpack

#endif
