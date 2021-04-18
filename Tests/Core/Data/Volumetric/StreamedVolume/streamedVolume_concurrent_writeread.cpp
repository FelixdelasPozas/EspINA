/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#include "Core/Analysis/Data/Volumetric/StreamedVolume.hxx"
#include "Core/Analysis/Data/Volumetric/WritableStreamedVolume.hxx"

#include <itkImageRegionIterator.h>

#include <QMutex>
#include <QThread>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace std;

template<typename T, unsigned int n>
class TestThread: public QThread
{
  public:
    TestThread(std::shared_ptr<WritableStreamedVectorVolume<T>> &data, int id, QMutex &mutex)
    : m_data(data)
    , m_id{id}
    , m_mutex(mutex)
    { m_mutex.lock(); }

  protected:
    void run()
    {
      auto region = m_data->itkRegion();
      auto dimension = T::GetImageDimension();

      for(int k = 0; k < 10; ++ k)
      {
        for(int i = region.GetIndex(dimension-1); i < region.GetIndex(dimension-1) + region.GetSize(dimension-1); ++i)
        {
          auto readRegion = region;
          readRegion.SetIndex(dimension-1, i);
          readRegion.SetSize(dimension-1, 1);

          qDebug() << "thread" << m_id << "reads slice" << i << "in loop" << k+1;
          auto image = m_data->read(readRegion);

          typename itk::ImageRegionIterator<T> it(image, image->GetLargestPossibleRegion());
          it.GoToBegin();
          while(!it.IsAtEnd())
          {
            auto value = it.Get();
            for(int j = 0; j < value.GetSize(); ++j)
            {
              value.SetElement(j, value.GetElement(j) + m_id/10.0);
            }

            it.Set(value);
            ++it;
          }

          qDebug() << "thread" << m_id << "writes slice" << i << "in loop" << k+1;
          m_data->draw(image);
        }
      }

      m_mutex.unlock();
    }

  private:
    std::shared_ptr<WritableStreamedVectorVolume<T>> &m_data;
    int m_id;
    QMutex &m_mutex;
};

int streamedVolume_concurrent_writeread( int argc, char** argv )
{
  int error = EXIT_SUCCESS;

  auto dir      = QDir::current();
  auto filename = dir.absoluteFilePath("test.mhd");
  auto info     = QFileInfo(filename);

  if(info.exists())
  {
    QFile::remove(info.absoluteFilePath());
    auto filename2 = dir.absoluteFilePath("test.raw");
    QFile::remove(filename2);
  }

  using RealVectorImageType = itk::VectorImage<double, 3>;

  try
  {
    RealVectorImageType::SpacingType spacing;
    spacing[0] = 1.1;
    spacing[1] = 2.2;
    spacing[2] = 3.3;

    RealVectorImageType::IndexType index;
    index[0] = 0;
    index[1] = 1;
    index[2] = 2;

    RealVectorImageType::SizeType size;
    size[0] = 100;
    size[1] = 100;
    size[2] = 100;

    RealVectorImageType::RegionType region;
    region.SetIndex(index);
    region.SetSize(size);

    auto creator = std::make_shared<WritableStreamedVectorVolume<RealVectorImageType>>(filename, region, spacing, 3);

    QMutex mutex1, mutex2, mutex3;
    auto thread1 = std::make_shared<TestThread<RealVectorImageType, 3>>(creator, 1, mutex1);
    auto thread2 = std::make_shared<TestThread<RealVectorImageType, 3>>(creator, 2, mutex2);
    auto thread3 = std::make_shared<TestThread<RealVectorImageType, 3>>(creator, 3, mutex3);
    thread1->start();
    thread2->start();
    thread3->start();

    mutex1.lock();
    mutex2.lock();
    mutex3.lock();

    // by now all threads have finished, there is no ability to lock a data until a thread has finished modifying
    // the slice so the values musn't be a certain amount, the only thing we can assure is that all the values
    // of a slice are the same.

    mutex3.unlock();
    mutex2.unlock();
    mutex1.unlock();

    auto reader = std::make_shared<StreamedVolume<RealVectorImageType>>(QFileInfo{filename});

    auto otherRegion = reader->itkRegion();
    auto dimension = RealVectorImageType::GetImageDimension();

    for(unsigned int i = region.GetIndex(dimension-1); i < region.GetIndex(dimension-1) + region.GetSize(dimension-1); ++i)
    {
      auto readRegion = otherRegion;
      readRegion.SetIndex(dimension-1, i);
      readRegion.SetSize(dimension-1, 1);

      auto image = reader->read(readRegion);

      typename itk::ImageRegionIterator<RealVectorImageType> it(image, image->GetLargestPossibleRegion());
      auto test = image->GetPixel(image->GetLargestPossibleRegion().GetIndex());
      std::cout << "test slice " << i << " test value: " << test << std::endl;
      it.GoToBegin();
      while(!it.IsAtEnd())
      {
        auto value = it.Get();
        for(unsigned int j = 0; j < value.GetSize(); ++j)
        {
          if(test.GetElement(j) != value.GetElement(j))
          {
            std::cout << "slice " << i << " has non-uniform values: " << value << " test is: " << test << std::endl;
            error = EXIT_FAILURE;
            break;
          }
        }
        ++it;
      }
    }
  }
  catch(const EspinaException &excp)
  {
    qDebug() << "exception:" << excp.what();
    qDebug() << "details:" << excp.details();
    error = EXIT_FAILURE;
  }
  catch(const itk::ExceptionObject &excp)
  {
    qDebug() << "exception:" << QString(excp.what());
    qDebug() << "details:" << QString(excp.GetDescription());
    qDebug() << "file:" << QString(excp.GetFile());
    qDebug() << "location:" << QString(excp.GetLocation());
    error = EXIT_FAILURE;
  }

  info.refresh();
  if(info.exists())
  {
    QFile::remove(info.absoluteFilePath());
    auto filename2 = dir.absoluteFilePath("test.raw");
    QFile::remove(filename2);
  }

  std::cout << "exit value: " << error << std::endl;

  return error;
}

