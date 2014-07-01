/*
 * 
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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
#include <Core/Utils/VolumeBounds.h>
#include "VolumetricData.h"

#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkSmartPointer.h>

#include <itkExtractImageFilter.h>
#include <itkImageToVTKImageFilter.h>

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
  typename T::RegionType equivalentRegion(const NmVector3& origin, const NmVector3& spacing, const Bounds& bounds);



  /** \brief Return the bounds for a given image region and an image
   * 
   * Bounds are given in nm using (0,0) as origin
   */
  template<typename T>
  Bounds equivalentBounds(const typename T::Pointer image, const typename T::RegionType& region);

  /** \brief Return the bounds for a given image region and an image
   *
   * Bounds are given in nm using (0,0) as origin
   */
  template<typename T>
  Bounds equivalentBounds(const NmVector3& origin, const NmVector3& spacing, const typename T::RegionType& region);



  /** \brief Return the minimum complete bounds for any image of given origin and spacing
   *
   */
  template<typename T>
  VolumeBounds volumeBounds(const NmVector3& origin, const NmVector3& spacing, const typename T::RegionType& region);

  /** \brief Return the minimum complete bounds for any image of given origin and spacing
   *
   */
  template<typename T>
  VolumeBounds volumeBounds(const typename T::Pointer image, const Bounds& bounds);

  /** \brief Return the minimum complete bounds for any image of given origin and spacing
   *
   */
  template<typename T>
  VolumeBounds volumeBounds(const typename T::Pointer image, const typename T::RegionType& region);

  /** \brief Return the minimum complete bounds for any image of given origin and spacing
   *
   */
  template<typename T>
  VolumeBounds volumeBounds(const NmVector3& origin, const NmVector3& spacing, const Bounds& bounds);


  /** \brief Return the bounds of the left slice of given @volume
   *
   */
  template<typename T>
  Bounds leftSliceBounds(const  T &volume);

  /** \brief Return the bounds of the right slice of given @volume
   *
   */
  template<typename T>
  Bounds rightSliceBounds(const  T &volume);

  /** \brief Return the bounds of the top slice of given @volume
   *
   */
  template<typename T>
  Bounds topSliceBounds(const  T &volume);

  /** \brief Return the bounds of the bottom slice of given @volume
   *
   */
  template<typename T>
  Bounds bottomSliceBounds(const  T &volume);

  /** \brief Return the bounds of the front slice of given @volume
   *
   */
  template<typename T>
  Bounds frontSliceBounds(const  T &volume);

  /** \brief Return the bounds of the back slice of given @volume
   *
   */
  template<typename T>
  Bounds backSliceBounds(const  T &volume);



  /** \brief Return the number of voxels in image whose value is @value
   *
   */
  template<typename T>
  unsigned long voxelCount(const typename T::Pointer image, const typename T::ValueType value);



  /** \brief Return the minimal bounds of image which contains voxels with values different from @value
   *
   */
  template<typename T>
  Bounds minimalBounds(const typename T::Pointer image, const typename T::ValueType value);



  /** \brief Transform NmVector to ItkSpacing
   *
   */
  template<typename T>
  typename T::SpacingType ItkSpacing(const NmVector3& spacing);



  /** \brief Transform from SpacingType to NmVector3
   *
   */
  template<typename T>
  NmVector3 ToNmVector3(typename T::SpacingType itkSpacing);

  /** \brief Transform from PointType to NmVector3
   *
   */
  template<typename T>
  NmVector3 ToNmVector3(typename T::PointType itkPoint);



  /** \brief Return the vtkImageData of specified bounds equivalent to the itkImage.
   */
  template<typename T>
  vtkSmartPointer<vtkImageData> vtkImage(const typename T::Pointer volume, const Bounds &inputBounds);

  /** \brief Return the vtkImageData of specified bounds equivalent to the
   *         volumetric data.
   */
  template<typename T>
  vtkSmartPointer<vtkImageData> vtkImage(VolumetricDataSPtr<T> volume, const Bounds &bounds);

  /** \brief Return the vtkImageData of specified bounds equivalent to the
   *         volumetric data of the specified output.
   */
  template<class T>
  vtkSmartPointer<vtkImageData> vtkImage(OutputSPtr output, const Bounds &bounds);



  /** \brief Volume's voxel's index at given spatial position
   * 
   *  It doesn't check whether the index is valid or not
   */
  template<typename T>
  typename T::IndexType index(Nm x, Nm y, Nm z);



  /** \brief Returns whether or not the voxel at @point is not background
   *
   */
  template<typename T>
  bool isSegmentationVoxel(const VolumetricDataSPtr<T> volume, const NmVector3 &point);



  /** \brief Draw @drawnVolume into @volume, resizing @volume bounds to fit @drawnVolume if necessary
   *
   */
  template<typename T>
  void expandAndDraw(VolumetricDataSPtr<T> volume, typename T::Pointer drawnVolume, Bounds bounds = Bounds());


  
  template<typename T>
  typename T::Pointer define_itkImage(const NmVector3              &origin  = {0, 0, 0},
                                      const NmVector3              &spacing = {1, 1, 1});

  template<typename T>
  typename T::Pointer create_itkImage(const Bounds&                 bounds,
                                      const typename T::ValueType   value   = 0,
                                      const NmVector3              &spacing = {1, 1, 1},
                                      const NmVector3              &origin  = {0, 0, 0});


  // NOTE: Probably move into an independent module
  template<typename T>
  double memory_size_in_MB(int number_of_pixels)
  {
    return number_of_pixels * sizeof(T) / 1024.0 / 1024.0;
  }


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
