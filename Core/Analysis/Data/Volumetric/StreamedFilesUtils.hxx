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

namespace ESPINA
{
  namespace Core
  {
    //-----------------------------------------------------------------------------
    template<typename T>
    VolumetricDataSPtr<T> readStreamedVolume(const QString &filename)
    {
      auto file = QFile{filename};

      if(!file.exists() || !filename.endsWith(".mhd", Qt::CaseInsensitive))
      {
        auto message = QObject::tr("Invalid file name or doesn't exist: %1.").arg(filename);
        auto details = QObject::tr("readStreamedVolume(filename) -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      if(!file.open(QIODevice::ReadOnly))
      {
        auto message = QObject::tr("Couldn't read mhd header file: %1").arg(filename);
        auto details = QObject::tr("readStreamedVolume(filename) -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      auto reader = itk::ImageFileReader<T>::New();
      reader->ReleaseDataFlagOn();
      reader->SetFileName(filename.toStdString());
      reader->UpdateOutputInformation();

      auto length = reader->GetOutput()->GetNumberOfComponentsPerPixel();

      return std::make_shared<StreamedVolume<T, length>>(QFileInfo{filename});
    }

    //-----------------------------------------------------------------------------
    template<typename T>
    VolumetricDataSPtr<T> readWritableStreamedVolume(const QString &filename)
    {
      auto file = QFile{filename};

      if(!file.exists() || !filename.endsWith(".mhd", Qt::CaseInsensitive))
      {
        auto message = QObject::tr("Invalid file name or doesn't exist: %1.").arg(filename);
        auto details = QObject::tr("readWritableStreamedVolume(filename) -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      if(!file.open(QIODevice::ReadOnly))
      {
        auto message = QObject::tr("Couldn't read mhd header file: %1").arg(filename);
        auto details = QObject::tr("readWritableStreamedVolume(filename) -> ") + message;

        throw Core::Utils::EspinaException(message, details);
      }

      auto reader = itk::ImageFileReader<T>::New();
      reader->ReleaseDataFlagOn();
      reader->SetFileName(filename.toStdString());
      reader->UpdateOutputInformation();

      auto length = reader->GetOutput()->GetNumberOfComponentsPerPixel();

      return std::make_shared<WritableStreamedVolume<T, length>>(QFileInfo{filename});
    }

  } // namespace Core
} // namespace ESPINA
