/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_VOLUMETRIC_DATA_UTILS_H
#define ESPINA_VOLUMETRIC_DATA_UTILS_H

#include <Core/Utils/Bounds.h>
#include <Core/Utils/NmVector3.h>

namespace EspINA
{
  /** \brief Return the image reagion equivalent to the bounds.
   * 
   * Different images may produce different regions for the same bounds.
   * Image origin and spacing are key values to obtain the image region
   * equivalent to a given bounds.
   *
   * TODO: how to proceed when bounds doesn't intersect with largest possible image region
   *       - we coud either return an invalid region or throw an execption
   */
  template<typename T>
  typename T::RegionType equivalentRegion(const T *image, const Bounds& bounds);

  /** \brief Return the bounds for a given image region
   * 
   * Bounds are given in nm using (0,0) as origin
   */
  template<typename T>
  Bounds equivalentBounds(const T *image, const typename T::RegionType& region);

  // NOTE: Probably move into an independent module
  static double memory_size_in_MB(int number_of_pixels){
    return number_of_pixels / 1024.0 / 1024.0;
  }

  template<typename T>
  typename T::Pointer create_itkImage(const Bounds&                 bounds,
                                      const typename T::ValueType   value   = 0,
                                      const typename T::SpacingType spacing = typename T::SpacingType(),
                                      const typename T::PointType   origin  = typename T::PointType());


//   /// Get the vtk-equivalent extent defining the volume
//   void extent(int out[6]) const = 0;


  /** \brief Volume's voxel's index at given spatial position
   * 
   *  It doesn't check whether the index is valid or not
   */   
  template<typename T>
  typename T::IndexType index( Nm x, Nm y, Nm z);

//   /// Set voxels at coordinates (x,y,z) to value
//   ///NOTE: Current implementation will expand the image
//   ///      when drawing with value != 0
//   void draw(Nm x, Nm y, Nm z,
//             itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
//             bool emitSignal = true) = 0;
// 
// 
//   /// Set voxels inside contour to value
//   ///NOTE: Current implementation will expand the image
//   ///      when drawing with value != 0
//   void draw(vtkPolyData *contour,
//             Nm slice,
//             PlaneType plane,
//             itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
//             bool emitSignal = true) = 0;
// 
//   //NOTE: To Deprecate?
//   /// Fill output's volume with given value
//   virtual void fill(itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
//                     bool emitSignal = true) = 0;
// 
//   /// Fill output's volume's region with given value
//   virtual void fill(const EspinaRegion &region,
//                     itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
//                     bool emitSignal = true) = 0;
// 
//   // NOTE: Needs to steal pointer from itkImage
//   virtual const vtkImageSPtr vtkImage() const = 0;
//   virtual const vtkImageSPtr vtkImage(const Bounds& bounds) const = 0;
// //     virtual itkVolumeIterator iterator() = 0;
// //     virtual itkVolumeIterator iterator(const EspinaRegion &region) = 0;
// // 
// //     virtual itkVolumeConstIterator constIterator() = 0;
// //     virtual itkVolumeConstIterator constIterator(const EspinaRegion &region) = 0;
// 
//   //TODO: Templatizate
// //   using VolumetricDataPtr  = VolumetricData*;
// //   using VolumetricDataSPtr = std::shared_ptr<VolumetricData>;
// // 
// //   VolumetricDataPtr  EspinaCore_EXPORT volumetricData(OutputPtr  output); //NOTE: Use viewitem??
// //   VolumetricDataSPtr EspinaCore_EXPORT volumetricData(OutputSPtr output);
// 
// 
// //     class EditedVolumeRegion
// //     : public Output::EditedRegion
// //     {
// //     public:
// //       EditedVolumeRegion(int id, const EspinaRegion &region)
// //       : EditedRegion(id, SegmentationVolume::TYPE, region){}
// // 
// //       virtual bool dump(QDir           cacheDir,
// //                         const QString &regionName,
// //                         Snapshot      &snapshot) const;
// // 
// //       itkVolumeType::Pointer Volume;
// //     };
// // 
// //     typedef boost::shared_ptr<EditedVolumeRegion> EditedVolumeRegionSPtr;
// //     typedef QList<EditedVolumeRegionSPtr>         EditedVolumeRegionSList;
// 
// 
// 
// 
//     virtual bool collision(SegmentationVolumeSPtr segmentation) = 0;
}

#include "Core/Analysis/Data/VolumetricDataUtils.txx"

#endif // ESPINA_VOLUMETRIC_DATA_UTILS_H
