/*
 * Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include "SparseVolume.h"
#include <Core/Utils/Bounds.h>

namespace EspINA {

const int UNSET = 0;
const int SET   = 1;

//-----------------------------------------------------------------------------
template<typename T>
SparseVolume<T>::SparseVolume()
: m_origin {0, 0, 0}
, m_spacing{1, 1, 1}
{
  setBackgroundValue(0);
}

//-----------------------------------------------------------------------------
template<typename T>
SparseVolume<T>::SparseVolume(const Bounds& bounds, const NmVector3& spacing)
: m_bounds{bounds}
, m_spacing{spacing}
{
  setBackgroundValue(0);
}

//-----------------------------------------------------------------------------
template<typename T>
double SparseVolume<T>::memoryUsage() const
{
  double numPixels = 0;

  for (unsigned int i = 0; i < m_blocks.size(); ++i)
    numPixels += m_blocks[i]->numberOfVoxels();

  return memory_size_in_MB(numPixels);
}


//-----------------------------------------------------------------------------
template<typename T>
Bounds SparseVolume<T>::bounds() const
{
  return m_bounds;
}

//-----------------------------------------------------------------------------
template<typename T>
void SparseVolume<T>::setOrigin(const NmVector3& origin)
{
  m_origin = origin;
}


//-----------------------------------------------------------------------------
template<typename T>
void SparseVolume<T>::setSpacing(const NmVector3& spacing)
{
  m_spacing = spacing;
}


//-----------------------------------------------------------------------------
template<typename T>
void SparseVolume<T>::setBlock(typename T::Pointer image)
{
  std::unique_ptr<Block> block(new SetBlock<T>(image));
  m_blocks.push_back(block);

  Bounds bounds = equivalentBounds<T>(image, image->GetLargestPossibleRegion());

  updateBlocksBoundingBox(bounds);
}


//-----------------------------------------------------------------------------
template<typename T>
void SparseVolume<T>::addBlock(BlockMaskUPtr mask)
{
  BlockUPtr block(new AddBlock(mask));
  m_blocks.push_back(block);

  updateBlocksBoundingBox(mask->bounds());
}


//-----------------------------------------------------------------------------
template<typename T>
void SparseVolume<T>::delBlock(BlockMaskUPtr mask)
{
  BlockUPtr block(new DelBlock(mask));
  m_blocks.push_back(block);

  updateBlocksBoundingBox(mask->bounds());
}


//-----------------------------------------------------------------------------
template<typename T>
const typename T::Pointer SparseVolume<T>::itkImage() const
{
  return itkImage(bounds());
}

//-----------------------------------------------------------------------------
template<typename T>
const typename T::Pointer SparseVolume<T>::itkImage(const Bounds& bounds) const
{
  if (!contains(this->bounds(), bounds)) throw Invalid_image_bounds();

  //auto image = create_itkImage<T>(bounds, backgroundValue(), m_spacing, m_origin);
  auto image = create_itkImage<T>(bounds, SEG_VOXEL_VALUE, m_spacing, m_origin);

  auto mask = createMask(bounds);

  int numPixels = image->GetLargestPossibleRegion().GetNumberOfPixels();

  // Transverse most recent blocks first while there are still requested pixels
  // that have not been set
  int i = m_blocks.size() - 1;
  while (i >= 0 && numPixels > 0)
  {
    Block *block = m_blocks[i].get();

    auto   blockBounds  = block->bounds(); //equivalentBounds(blockImage, blockImage->GetLargestPossibleRegion());
    Bounds commonBounds = intersection(bounds, blockBounds);

    bool validBounds = commonBounds.areValid();
    for (int i = 0, j = 0; i < 6; i +=2, j +=2) {
      validBounds &= !(commonBounds[i] == commonBounds[j] && !commonBounds.areLowerIncluded(toAxis(i)) && !commonBounds.areUpperIncluded(toAxis(i)));
    }
    //std::cout << "Check Block Intersection: " << bounds << " ∩ " << blockBounds << " = " << commonBounds << std::endl;
    if (validBounds) {
      //      updateCommonPixels(block, commonBounds, image, mask, numPixels);
    }
    --i;
  }

  return image;
}


//-----------------------------------------------------------------------------
template<typename T>
void SparseVolume<T>::draw(const vtkImplicitFunction  *brush,
                           const Bounds               &bounds,
                           const typename T::ValueType value)
{
//   //cout << "Volume bounds" << m_bounds << endl;
//   //cout << "Requested bounds" << bounds << endl;
//   Bounds blockBounds = intersection(bounds, m_bounds);
//   if (blockBounds.areValid()) {
// 
//     BlockMaskUPtr mask{new BlockMask(blockBounds)};//ImageType::Pointer blockImage = create_itkImage(blockBounds, m_bgValue, m_spacing, m_origin);
// 
//     //cout << "Block bounds" << blockBounds << endl;
//     //blockImage->GetLargestPossibleRegion().Print(cout);
//     //cout << "Number of block Pixels:" << blockImage->GetLargestPossibleRegion().GetNumberOfPixels() << endl;
// 
//     bool drawOp = value != backgroundValue();
// 
//     ImageIterator it = ImageIterator(blockImage, equivalentRegion(blockImage, blockBounds));
//     for (it.GoToBegin(); !it.IsAtEnd(); ++it )
//     {
//       double tx = it.GetIndex()[0]*m_spacing[0] + m_origin[0];
//       double ty = it.GetIndex()[1]*m_spacing[1] + m_origin[1];
//       double tz = it.GetIndex()[2]*m_spacing[2] + m_origin[2];
// 
//       if (brush->FunctionValue(tx, ty, tz) <= 0)
//         it.Set(maskValue);
//     }
// 
//     if (drawOp) {
//       addBlock(blockImage, value);
//     } else {
//       delBlock(blockImage);
//     }
//   }
}


//-----------------------------------------------------------------------------
template<typename T>
void SparseVolume<T>::resize(const Bounds &bounds)
{
  m_bounds = bounds;
  m_blocks_bounding_box = intersection(m_bounds, m_blocks_bounding_box);

  // TODO: Reduce existing blocks
//   for (auto block : m_blocks) {
//     
//   }
}

//-----------------------------------------------------------------------------
template<typename T>
bool SparseVolume<T>::isValid() const
{
  return m_bounds.areValid();
}

//-----------------------------------------------------------------------------
template<typename T>
Snapshot SparseVolume<T>::snapshot() const
{
  Snapshot snapshot;

  return snapshot;
}

//-----------------------------------------------------------------------------
template<typename T>
typename SparseVolume<T>::BlockMaskUPtr SparseVolume<T>::createMask(const Bounds& bounds) const
{
  BlockMask::itkSpacing spacing;
//  BlockMaskUPtr mask(new BlockMask(bounds, spacing));

//   mask->SetRegions(image->GetLargestPossibleRegion());
//   mask->Allocate();
//   mask->FillBuffer(0);

  return BlockMaskUPtr();
}

// bool SparseVolume::updatePixel(const BlockType op, ImageIterator bit, ImageIterator itt) const
// {
//   bool updated = false;
//   switch (op) {
//     case BlockType::ADD:
//       if (bit.Get() != m_bgValue) {
//         itt.Set(bit.Get());
//         updated = true;
//       }
//       break;
//     case BlockType::DEL:
//       if (bit.Get() != m_bgValue) {
//         itt.Set(m_bgValue);
//         updated = true;
//       }
//       break;
//     case BlockType::SET:
//       itt.Set(m_bgValue);
//       updated = true;
//       break;
//   }
// 
//   return updated;
// }
// 
// //-----------------------------------------------------------------------------
// void SparseVolume::updateCommonPixels(const BlockType op, const Bounds& bounds, const ImageType::Pointer block, ImageType::Pointer image, ImageType::Pointer mask, int& remainingPixels) const
// {
//   ImageType::RegionType commonBlockRegion = equivalentRegion(block, bounds);
//   ImageType::RegionType commonImageRegion = equivalentRegion(image, bounds);
// 
//   ImageIterator iit = ImageIterator(image, commonImageRegion); // image iterator
//   ImageIterator mit = ImageIterator(mask,  commonImageRegion); // mask  iterator
//   ImageIterator bit = ImageIterator(block, commonBlockRegion); // block iterator
// 
//   iit.GoToBegin(); mit.GoToBegin(); bit.GoToBegin();
//   while (remainingPixels > 0 && !iit.IsAtEnd())
//   {
//     if (mit.Get() == UNSET && updatePixel(op, bit, iit))
//     {
//       --remainingPixels;
//       mit.Set(SET);
//     }
//     ++iit;
//     ++mit;
//     ++bit;
//   }
// }

//-----------------------------------------------------------------------------
template<typename T>
void SparseVolume<T>::updateBlocksBoundingBox(const Bounds& bounds)
{
  if (m_blocks_bounding_box.areValid()) {
    m_blocks_bounding_box = boundingBox(m_blocks_bounding_box, bounds);
  } else {
    m_blocks_bounding_box = bounds;
  }
}

}
