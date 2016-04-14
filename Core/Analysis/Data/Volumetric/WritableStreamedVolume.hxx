/*

 Copyright (C) 2016 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 */

#ifndef CORE_ANALYSIS_DATA_VOLUMETRIC_WRITABLESTREAMEDVOLUME_HXX_
#define CORE_ANALYSIS_DATA_VOLUMETRIC_WRITABLESTREAMEDVOLUME_HXX_

// ESPINA
#include <Core/Utils/BinaryMask.hxx>
#include <Core/Analysis/Data/Volumetric/StreamedVolume.hxx>

// VTK
#include <vtkImplicitFunction.h>

// ITK
#include <itkImage.h>
#include <itkNumericTraitsVectorPixel.h>
#include <itkVectorImage.h>

namespace ESPINA
{
  namespace Core
  {
    /** \class FileWriter
     * \brief Helper class to write image regions to a big image on disk.
     *
     * NOTE: this is just a trick to avoid partial specialization of the
     *       WrittableStreamedVolume class for vector length of 1. As
     *       methods can't be partially specialized.
     *
     */
    template<typename T, unsigned int VLenght>
    class FileWriter
    {
      public:
        void write(const QString &fileName, const typename T::Pointer &image);
    };

    /** \class FileWriter specialization
     * \brief Helper class to write image regions to a big image on disk.
     *
     */
    template<typename T>
    class FileWriter<T,1>
    {
      public:
        void write(const QString &fileName, const typename T::Pointer &image);
    };

    /** \class WritableStreamedVolume
     * \brief Wrapper class around itk::Image to create and read/write data files.
     *        Components per pixel different from 1 are supported on
     *        construction with the VLength template parameter.
     *        Default vector size is 1, equivalent to regular images.
     *
     */
    template<typename T, unsigned int VLength = 1>
    class WritableStreamedVolume
    : public StreamedVolume<T,VLength>
    {
      public:
        /** \brief WritableStreamedVolume class constructor.
         * \param[in] fileName filename of the disk data file to be created.
         * \param[in] region itk::Image region
         * \param[in] spacing itk::Image spacing
         *
         * NOTE: if the file exists the region and spacing can be empty.
         *
         */
        explicit WritableStreamedVolume(const QFileInfo               &fileName,
                                        const typename T::RegionType  &region = typename T::RegionType(),
                                        const typename T::SpacingType &spacing = typename T::SpacingType());

        /** \brief WritableStreamedVolume class virtual destructor.
         *
         */
        virtual ~WritableStreamedVolume()
        {}

        virtual void setOrigin(const NmVector3& origin) override;

        virtual void setSpacing(const NmVector3& spacing) override;

        virtual void draw(vtkImplicitFunction*        brush,
                          const Bounds&               bounds,
                          const typename T::ValueType value) override;

        virtual void draw(const typename T::Pointer volume) override;

        virtual void draw(const typename T::Pointer volume,
                          const Bounds&             bounds) override;

        virtual void draw(const typename T::IndexType &index,
                          const typename T::PixelType  value = SEG_VOXEL_VALUE) override;

        virtual void draw(const Bounds               &bounds,
                          const typename T::PixelType value = SEG_VOXEL_VALUE) override;

        virtual void draw(const BinaryMaskSPtr<typename T::ValueType> mask,
                          const typename T::ValueType value = SEG_VOXEL_VALUE) override;

        virtual void write(const typename T::Pointer &image) override;

      private:
        FileWriter<T,VLength> m_writer;
    };

    //-----------------------------------------------------------------------------
    template<typename T, unsigned int VLength>
    void FileWriter<T, VLength>::write(const QString &fileName, const typename T::Pointer &image)
    {
      // Direct write to modify the region on disk.
      if(!QFile::exists(fileName))
      {
        auto message = QObject::tr("Filename '%1' doesn't exist.").arg(fileName);
        auto details = QObject::tr("FileWriter<T,N>::write(fileName, image) -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      auto dataFile = fileName;
      dataFile = dataFile.replace(".mhd", ".raw");

      QFile file{dataFile};
      if(!file.open(QIODevice::ReadWrite))
      {
        auto message = QObject::tr("Couldn't open raw file '%1'. Reason: %2.").arg(dataFile).arg(file.errorString());
        auto details = QObject::tr("FileWriter<T,N>::write(fileName, image) -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      // slow due to not knowing beforehand the dimensions of the image.
      auto dataSize = sizeof(typename T::InternalPixelType) * VLength;
      auto region   = image->GetLargestPossibleRegion();
      auto size     = region.GetSize();
      auto it       = itk::ImageRegionConstIteratorWithIndex<T>(image, region);

      it.GoToBegin();
      while(!it.IsAtEnd())
      {
        long long position = 0;
        auto index = it.GetIndex();
        auto accumulator = 1;

        for(unsigned int i = 0; i < T::GetImageDimension(); ++i)
        {
          position += index[i] * accumulator;
          accumulator *= size[i];
        }
        position *= dataSize;

        typename T::PixelType value = it.Get();
        if(!file.seek(position))
        {
          auto message = QObject::tr("Unable to seek to pos %1, total file size is %2. Filename: %3").arg(position).arg(file.size()).arg(fileName);
          auto details = QObject::tr("FileWriter<T,N>::write(fileName, image) -> ") + message;

          throw Core::Utils::EspinaException(message, details);
        }

        if(dataSize != file.write(reinterpret_cast<const char *>(value.GetDataPointer()), dataSize))
        {
          auto message = QObject::tr("Unable to write in pos %1, total file size is %2. Filename: %3").arg(position).arg(file.size()).arg(fileName);
          auto details = QObject::tr("FileWriter<T,N>::write(fileName, image) -> ") + message;

          throw Core::Utils::EspinaException(message, details);
        }

        ++it;
      }

      if(!file.flush() || (file.error() != QFile::NoError))
      {
        auto message = QObject::tr("Error finishing write operation in file: %3").arg(fileName);
        auto details = QObject::tr("FileWriter<T,N>::write(fileName, image) -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }
      file.close();
    }

    //-----------------------------------------------------------------------------
    template<typename T>
    void FileWriter<T, 1>::write(const QString &fileName, const typename T::Pointer &image)
    {
      // Direct write to modify the region on disk.
      if(!QFile::exists(fileName))
      {
        auto message = QObject::tr("Filename '%1' doesn't exist.").arg(fileName);
        auto details = QObject::tr("FileWriter<T,1>::write(fileName, image) -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      auto dataFile = fileName;
      dataFile = dataFile.replace(".mhd", ".raw");

      QFile file{dataFile};
      if(!file.open(QIODevice::ReadWrite))
      {
        auto message = QObject::tr("Couldn't open raw file '%1'. Reason: %2.").arg(dataFile).arg(file.errorString());
        auto details = QObject::tr("FileWriter<T,1>::write(fileName, image) -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      // slow due to not knowing beforehand the dimensions of the image.
      auto dataSize = sizeof(typename T::InternalPixelType);
      auto region   = image->GetLargestPossibleRegion();
      auto size     = region.GetSize();
      auto it       = itk::ImageRegionConstIteratorWithIndex<T>(image, region);

      it.GoToBegin();
      while(!it.IsAtEnd())
      {
        long long position = 0;
        auto index = it.GetIndex();
        auto accumulator = 1;

        for(unsigned int i = 0; i < T::GetImageDimension(); ++i)
        {
          position += index[i] * accumulator;
          accumulator *= size[i];
        }

        auto value = it.Get();
        if(!file.seek(position))
        {
          auto message = QObject::tr("Unable to seek to pos %1, total file size is %2. Filename: %3").arg(position).arg(file.size()).arg(fileName);
          auto details = QObject::tr("FileWriter<T,1>::write(fileName, image) -> ") + message;

          throw Core::Utils::EspinaException(message, details);
        }

        if(dataSize != file.write(reinterpret_cast<const char *>(&value), dataSize))
        {
          auto message = QObject::tr("Unable to write in pos %1, total file size is %2. Filename: %3").arg(position).arg(file.size()).arg(fileName);
          auto details = QObject::tr("FileWriter<T,1>::write(fileName, image) -> ") + message;

          throw Core::Utils::EspinaException(message, details);
        }

        ++it;
      }

      if(!file.flush() || (file.error() != QFile::NoError))
      {
        auto message = QObject::tr("Error finishing write operation in file: %3").arg(fileName);
        auto details = QObject::tr("FileWriter<T,1>::write(fileName, image) -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }
      file.close();
    }

    //-----------------------------------------------------------------------------
    template<typename T, unsigned int VLength>
    WritableStreamedVolume<T, VLength>::WritableStreamedVolume(const QFileInfo &fileName, const typename T::RegionType &region, const typename T::SpacingType &spacing)
    : StreamedVolume<T, VLength>()
    {
      this->m_fileName = fileName.absoluteFilePath();

      typename T::PixelType pixelType;
      this->setBackgroundValue(itk::NumericTraits<typename T::PixelType>::ZeroValue(pixelType));

      if(fileName.exists())
      {
        // discards region and spacing parameters
        auto reader = itk::ImageFileReader<T>::New();
        reader->ReleaseDataFlagOn();
        reader->SetFileName(this->m_fileName.toStdString());
        reader->UpdateOutputInformation();

        typename T::Pointer image = reader->GetOutput();

        this->m_origin  = image->GetOrigin();
        this->m_spacing = image->GetSpacing();
        this->m_region  = image->GetLargestPossibleRegion();

        // region must be updated because all regions in EspINA have an implicit origin of {0,0,0}
        for(int i = 0; i < T::GetImageDimension(); ++i)
        {
          this->m_region.SetIndex(i, vtkMath::Round(this->m_origin.GetElement(i)/this->m_spacing.GetElement(i)));
        }

        if(image->GetNumberOfComponentsPerPixel() != VLength)
        {
          auto message = QObject::tr("Invalid number of components per pixel (vector size).");
          auto details = QObject::tr("WritableStreamedVolume::constructor() -> ") + message;

          throw Core::Utils::EspinaException(message, details);
        }
      }
      else
      {
        for(int i = 0; i < T::GetImageDimension(); ++i)
        {
          this->m_origin.SetElement(i, region.GetIndex(i)*spacing.GetElement(i));
        }
        this->m_spacing = spacing;
        this->m_region  = region;

        if(this->m_spacing == typename T::SpacingType())
        {
          auto message = QObject::tr("Invalid parameters: emtpy spacing.");
          auto details = QObject::tr("WritableStreamingVolume<> -> ") + message;

          throw Core::Utils::EspinaException(message, details);
        }

        if(this->m_region == typename T::RegionType())
        {
          auto message = QObject::tr("Invalid parameters: empty region.");
          auto details = QObject::tr("WritableStreamingVolume<> -> ") + message;

          throw Core::Utils::EspinaException(message, details);
        }

        auto image = T::New();

        // Initial mhd file is just a voxel, we'll expand the data file later using Qt file interface.
        // We're just avoiding the allocation of the whole image on RAM.
        typename T::RegionType fakeRegion;
        fakeRegion.SetIndex(region.GetIndex());
        for(int i = 0; i < T::GetImageDimension(); ++i)
        {
          fakeRegion.SetSize(i, 1);
        }

        image->SetRegions(fakeRegion);
        image->SetSpacing(spacing);
        if(VLength != 1)
        {
          image->SetNumberOfComponentsPerPixel(VLength);
        }
        image->Allocate();
        image->Update();

        auto writer = itk::ImageFileWriter<T>::New();
        writer->SetFileName(this->m_fileName.toStdString());
        writer->SetInput(image);
        writer->SetImageIO(itk::MetaImageIO::New());
        writer->Update();

        QFile headerFile{this->m_fileName};
        if(!headerFile.open(QIODevice::ReadWrite))
        {
          auto message = QObject::tr("Couldn't open header file: %1, error: %2").arg(this->m_fileName).arg(headerFile.errorString());
          auto details = QObject::tr("WritableStreamedVolume::WritableStreamedVolume() -> ") + message;

          throw Core::Utils::EspinaException(message, details);
        }

        auto contents = headerFile.readAll();
        auto replacement = QString("DimSize =");
        for(int i = 0; i < T::GetImageDimension(); ++i)
        {
          replacement.append(QObject::tr(" %1").arg(region.GetSize(i)));
        }

        auto beginPos = contents.indexOf("DimSize =");
        auto endPos   = contents.indexOf('\n', beginPos);
        contents.replace(beginPos, endPos-beginPos, replacement.toStdString().c_str());

        headerFile.seek(0);
        headerFile.write(contents);
        headerFile.close();

        if(headerFile.error() != QFile::NoError)
        {
          auto message = QObject::tr("Couldn't close header file: %1, error: %2").arg(this->m_fileName).arg(headerFile.errorString());
          auto details = QObject::tr("WritableStreamedVolume::WritableStreamedVolume() -> ") + message;

          throw Core::Utils::EspinaException(message, details);
        }

        auto dataFilename = headerFile.fileName().replace(".mhd", ".raw", Qt::CaseInsensitive);

        QFile dataFile{dataFilename};
        if(!dataFile.open(QIODevice::WriteOnly|QIODevice::Truncate))
        {
          auto message = QObject::tr("Couldn't open data file: %1, error: %2").arg(dataFilename).arg(dataFile.errorString());
          auto details = QObject::tr("WritableStreamedVolume::WritableStreamedVolume() -> ") + message;

          throw Core::Utils::EspinaException(message, details);
        }

        long long totalsize = 1;
        for(int i = 0; i < T::GetImageDimension(); ++i)
        {
          totalsize *= region.GetSize(i);
        }
        totalsize *= sizeof(typename T::InternalPixelType) * VLength;

        // according to Qt this zeroes the contents of the file.
        if(!dataFile.resize(totalsize))
        {
          auto message = QObject::tr("Couldn't resize data file: %1 to %2 bytes in size, error: %3").arg(dataFilename).arg(totalsize).arg(dataFile.errorString());
          auto details = QObject::tr("WritableStreamedVolume::WritableStreamedVolume() -> ") + message;

          throw Core::Utils::EspinaException(message, details);
        }

        dataFile.flush();
        dataFile.close();

        if(dataFile.error() != QFile::NoError)
        {
          auto message = QObject::tr("Couldn't close data file: %1, error: %2").arg(dataFile.fileName()).arg(dataFile.errorString());
          auto details = QObject::tr("WritableStreamedVolume::WritableStreamedVolume() -> ") + message;

          throw Core::Utils::EspinaException(message, details);
        }
      }
    }

    //------------------------------------------------------------------------
    template<class T, unsigned int VLength>
    inline void WritableStreamedVolume<T, VLength>::setOrigin(const NmVector3 &origin)
    {
      // use of NmVector3 limits to 3 dimensions.
      if(T::GetImageDimension() != 3)
      {
        auto message = QObject::tr("Image dimension is not 3.");
        auto details = QObject::tr("WritableStreamedVolume::setOrigin() -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      for(int i = 0; i < T::GetImageDimension(); ++i)
      {
        this->m_origin.SetElement(i, origin[i]);
        this->m_region.SetIndex(i, vtkMath::Round(origin[i]/this->m_spacing.GetElement(i)));
      }

      QFile headerFile{this->m_fileName};
      if (!headerFile.open(QIODevice::ReadWrite))
      {
        auto message = QObject::tr("Couldn't open header file: %1, error: %2").arg(this->m_fileName).arg(headerFile.errorString());
        auto details = QObject::tr("WritableStreamedVolume::setOrigin() -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      auto contents = headerFile.readAll();
      auto replacement = QString("Offset =");
      for (int i = 0; i < T::GetImageDimension(); ++i)
      {
        replacement.append(' ');
        replacement.append(QObject::tr("%1").arg(this->m_origin.GetElement(i)));
      }

      auto beginPos = contents.indexOf("Offset =");
      auto endPos = contents.indexOf('\n', beginPos);
      contents.replace(beginPos, endPos - beginPos, replacement.toStdString().c_str());

      headerFile.seek(0);
      headerFile.write(contents);
      headerFile.close();

      if (headerFile.error() != QFile::NoError)
      {
        auto message = QObject::tr("Couldn't close header file: %1, error: %2").arg(this->m_fileName).arg(headerFile.errorString());
        auto details = QObject::tr("WritableStreamedVolume::setOrigin() -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }
    }

    //------------------------------------------------------------------------
    template<class T, unsigned int VLength>
    inline void WritableStreamedVolume<T, VLength>::setSpacing(const NmVector3 &spacing)
    {
      // use of NmVector3 limits to 3 dimensions.
      if(T::GetImageDimension() != 3)
      {
        auto message = QObject::tr("Image dimension is not 3.");
        auto details = QObject::tr("WritableStreamedVolume::setSpacing() -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      this->m_spacing.SetElement(0, spacing[0]);
      this->m_spacing.SetElement(1, spacing[1]);
      this->m_spacing.SetElement(2, spacing[2]);

      QFile headerFile{this->m_fileName};
      if (!headerFile.open(QIODevice::ReadWrite))
      {
        auto message = QObject::tr("Couldn't open header file: %1, error: %2").arg(this->m_fileName).arg(headerFile.errorString());
        auto details = QObject::tr("WritableStreamedVolume::setSpacing() -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      auto contents = headerFile.readAll();
      auto replacement = QString("ElementSpacing =");
      for (int i = 0; i < T::GetImageDimension(); ++i)
      {
        replacement.append(' ');
        replacement.append(QObject::tr("%1").arg(this->m_spacing.GetElement(i)));
      }

      auto beginPos = contents.indexOf("ElementSpacing =");
      auto endPos = contents.indexOf('\n', beginPos);
      contents.replace(beginPos, endPos - beginPos, replacement.toStdString().c_str());

      headerFile.seek(0);
      headerFile.write(contents);
      headerFile.close();

      if (headerFile.error() != QFile::NoError)
      {
        auto message = QObject::tr("Couldn't close header file: %1, error: %2").arg(this->m_fileName).arg(headerFile.errorString());
        auto details = QObject::tr("WritableStreamedVolume::setSpacing() -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }
    }

    //------------------------------------------------------------------------
    template<class T, unsigned int VLength>
    inline void WritableStreamedVolume<T, VLength>::write(const typename T::Pointer &image)
    {
      auto volumeRegion = image->GetLargestPossibleRegion();
      auto volumeOrigin = image->GetOrigin();

      if(!this->m_region.IsInside(volumeRegion))
      {
        auto message = QObject::tr("Image region partially or completely outside of the large image region.");
        auto details = QObject::tr("StreamedVolume::write() -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      auto origin = volumeOrigin;
      auto region = volumeRegion;
      for(int i = 0; i < T::GetImageDimension(); ++i)
      {
        region.SetIndex(i, region.GetIndex(i)-this->m_region.GetIndex(i));
        origin.SetElement(i, this->m_region.GetIndex(i)*this->m_spacing.GetElement(i));
      }

      QMutexLocker lock(&this->m_lock);

      image->SetOrigin(origin);
      image->SetRegions(region);
      image->UpdateOutputInformation();

      this->m_writer.write(this->m_fileName, image);

      // image may be needed later, reset the original values.
      image->SetOrigin(volumeOrigin);
      image->SetRegions(volumeRegion);
      image->UpdateOutputInformation();
    }

    //-----------------------------------------------------------------------------
    template<typename T, unsigned int VLength>
    void WritableStreamedVolume<T, VLength>::draw(const typename T::Pointer volume)
    {
      auto volumeRegion = volume->GetLargestPossibleRegion();

      typename T::Pointer drawVolume = volume;

      if(!this->m_region.IsInside(volumeRegion))
      {
        volumeRegion.Crop(this->m_region);
        qWarning() << "StreamedVolume::draw(volume) -> asked for a region partially outside the image region.\n";

        drawVolume = extract_image<T>(volume, volumeRegion);
      }

      write(drawVolume);
    }

    //-----------------------------------------------------------------------------
    template<typename T, unsigned int VLength>
    void WritableStreamedVolume<T, VLength>::draw(const typename T::Pointer volume, const Bounds& bounds)
    {
      NmVector3 origin{this->m_origin[0], this->m_origin[1], this->m_origin[2]};
      NmVector3 spacing{this->m_spacing[0], this->m_spacing[1], this->m_spacing[2]};

      auto volumeRegion = equivalentRegion<T>(origin, spacing, bounds);
      auto drawVolume   = extract_image<T>(volume, volumeRegion);

      if(!this->m_region.IsInside(volumeRegion))
      {
        volumeRegion.Crop(this->m_region);
        qWarning() << "StreamedVolume::draw(volume, bounds) -> asked for a region partially outside the image region.\n";

        drawVolume = extract_image<T>(volume, volumeRegion);
      }

      write(drawVolume);
    }

    //-----------------------------------------------------------------------------
    template<typename T, unsigned int VLength>
    void WritableStreamedVolume<T, VLength>::draw(const Bounds &bounds, const typename T::PixelType value)
    {
      NmVector3 origin{this->m_origin[0], this->m_origin[1], this->m_origin[2]};
      NmVector3 spacing{this->m_spacing[0], this->m_spacing[1], this->m_spacing[2]};

      auto volumeRegion = equivalentRegion<T>(origin, spacing, bounds);

      if(!this->m_region.IsInside(volumeRegion))
      {
        volumeRegion.Crop(this->m_region);
        qWarning() << "StreamedVolume::draw(bounds, pixelValue) -> asked for a region partially outside the image region.\n";
      }

      auto volume  = T::New();
      volume->SetRegions(volumeRegion);
      volume->SetSpacing(this->m_spacing);
      if(VLength > 1)
      {
        volume->SetNumberOfComponentsPerPixel(VLength);
      }
      volume->Allocate();
      volume->FillBuffer(value);

      write(volume);
    }

    //-----------------------------------------------------------------------------
    template<typename T, unsigned int VLength>
    void WritableStreamedVolume<T, VLength>::draw(const typename T::IndexType &index, const typename T::PixelType value)
    {
      typename T::RegionType pixelRegion;
      pixelRegion.SetIndex(index);
      for(int i = 0; i < T::GetImageDimension(); ++i)
      {
        pixelRegion.SetSize(i, 1);
      }

      auto volume = T::New();
      volume->SetRegions(pixelRegion);
      volume->SetSpacing(this->m_spacing);
      if(VLength > 1)
      {
        volume->SetNumberOfComponentsPerPixel(VLength);
      }
      volume->Allocate();
      volume->FillBuffer(value);

      write(volume);
    }

    //-----------------------------------------------------------------------------
    template<typename T, unsigned int VLength>
    void WritableStreamedVolume<T, VLength>::draw(vtkImplicitFunction*   brush,
                                             const Bounds&               bounds,
                                             const typename T::ValueType value)
    {
      // use of bounds limited to 3 dimensions
      if(T::GetImageDimension() != 3)
      {
        auto message = QObject::tr("Image dimension is not 3.");
        auto details = QObject::tr("WritableStreamedVolume::draw(implicit, bounds, value) -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      auto image = this->itkImage(bounds);

      itk::ImageRegionIteratorWithIndex<T> it(image, image->GetLargestPossibleRegion());
      it.GoToBegin();
      while(!it.IsAtEnd())
      {
        auto index = it.GetIndex();
        auto functionValue = brush->FunctionValue(index.GetElement(0)*this->m_spacing.GetElement(0),
                                                  index.GetElement(1)*this->m_spacing.GetElement(1),
                                                  index.GetElement(2)*this->m_spacing.GetElement(2));
        if(functionValue <= 0)
        {
          it.Set(value);
        }
        ++it;
      }

      write(image);
    }

    //-----------------------------------------------------------------------------
    template<typename T, unsigned int VLength>
    void WritableStreamedVolume<T, VLength>::draw(const BinaryMaskSPtr<typename T::ValueType> mask,
                                                  const typename T::ValueType                 value)
    {
      // use of bounds and binary mask limited to 3 dimensions
      if(T::GetImageDimension() != 3)
      {
        auto message = QObject::tr("Image dimension is not 3.");
        auto details = QObject::tr("WritableStreamedVolume::draw(mask, value) -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      auto bounds = mask->bounds().bounds();
      auto image  = this->itkImage(bounds);

      itk::ImageRegionIteratorWithIndex<T> iit(image, image->GetLargestPossibleRegion());
      typename BinaryMask<typename T::ValueType>::const_region_iterator mit(mask.get(), bounds);
      iit.GoToBegin();
      while(!iit.IsAtEnd())
      {
        if(mit.isSet())
        {
          iit.Set(value);
        }

        ++iit;
        ++mit;
      }

      write(image);
    }

  } // namespace Core
} // namespace ESPINA

#endif // CORE_ANALYSIS_DATA_VOLUMETRIC_WRITABLESTREAMEDVOLUME_HXX_
