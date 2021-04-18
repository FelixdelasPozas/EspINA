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

// Project
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <Core/IO/ZipUtils.h>
#include <Core/Utils/EspinaException.h>
#include <Core/Utils/SpatialUtils.hxx>
#include <Core/Types.h>
#include <itkImageFileWriter.h>

// Qt
#include <QFileDialog>
#include <QDebug>

// Quazip
#include <quazip/quazip.h>
#include <ToolsDev/fakeStacks/FakeStacks.h>

using namespace ESPINA;
using namespace ESPINA::IO;
using namespace ESPINA::Core::Utils;

const QString VERSION_FILE        = "formatInfo.ini";
const QString CONTENT_FILE        = "content.dot";
const QString RELATIONS_FILE      = "relations.dot";

//-----------------------------------------------------------------
FakeStacks::FakeStacks(QWidget *parent, Qt::WindowFlags flags)
: QDialog{parent, flags}
{
  setupUi(this);

  connect(m_add, SIGNAL(clicked()), this, SLOT(addFiles()));
  connect(m_quit, SIGNAL(clicked()), this, SLOT(close()));
  connect(m_start, SIGNAL(clicked()), this, SLOT(startGeneration()));

  m_start->setEnabled(false);
  m_progress->setEnabled(false);
  m_progress->setValue(0);
  m_progress->setMaximum(100);

  setFixedWidth(1000);
}

//-----------------------------------------------------------------
FakeStacks::~FakeStacks()
{
}

//-----------------------------------------------------------------
void FakeStacks::addFiles()
{
  if(!m_files.isEmpty())
  {
    writeInfo(tr("Cleared %1 files").arg(m_files.size()));
    m_files.clear();
  }

  auto files = QFileDialog::getOpenFileNames(this, tr("Open SEG files"), QDir::currentPath(), tr("*.seg"));

  if(!files.empty())
  {
    for(auto file: files)
    {
      auto fileInfo = QFileInfo{file};

      if(fileInfo.exists() && file.endsWith(".seg", Qt::CaseInsensitive))
      {
        m_files << fileInfo;
        m_dirs.insert(fileInfo.fileName(), fileInfo.path());

        writeImportant(tr("Added file: %1").arg(fileInfo.fileName()));

        m_start->setEnabled(true);
      }
    }
  }
}

//-----------------------------------------------------------------
void FakeStacks::startGeneration()
{
  m_progress->setEnabled(true);
  m_progress->setValue(0);
  m_progress->setMaximum(100);

  double increment = 1/static_cast<double>(m_files.size());
  double progress = 0;

  writeInfo(" ");

  for(auto file: m_files)
  {
    m_stacks.clear();
    m_ids.clear();
    m_spacing.clear();

    QApplication::processEvents();

    QFileInfo fileInfo{file};
    if(!fileInfo.exists())
    {
      writeError(tr("ERROR: %1 doesn't exists").arg(fileInfo.absoluteFilePath()));
      continue;
    }

    writeImportant(tr("Processing: %1").arg(file.fileName()));

    QuaZip sourceZip{fileInfo.absoluteFilePath()};
    sourceZip.open(QuaZip::mdUnzip, nullptr);

    if(sourceZip.getZipError() != UNZ_OK)
    {
      writeError(tr("ERROR: couldn't open file: %1. Cause: %2").arg(fileInfo.fileName()).arg(sourceZip.getZipError()));
      progress += static_cast<double>(1/m_files.size())*100;
      m_progress->setValue(progress);
      continue;
    }

    try
    {
      if(!isVersion5or6file(ZipUtils::readFileFromZip(VERSION_FILE, sourceZip)))
      {
        writeError(tr("ERROR: file '%1' is not SEG file version 6.").arg(file.fileName()));
        continue;
      }
    }
    catch(const EspinaException &e)
    {
      writeError(tr("ERROR: Error uncompressing file: %1").arg(VERSION_FILE));
      writeError(tr("CAUSE: %1").arg(e.details()));
      continue;
    }

    try
    {
      QByteArray contents = ZipUtils::readFileFromZip(CONTENT_FILE, sourceZip);

      parseStacks(contents);

      if(m_ids.empty())
      {
        writeError(tr("ERROR: Can't find stacks in 'content.dot'"));
        continue;
      }

      parseBounds(sourceZip);
    }
    catch(const EspinaException &e)
    {
      writeError(tr("ERROR: Error uncompressing file: %1").arg(CONTENT_FILE));
      writeError(tr("CAUSE: %1").arg(e.details()));
      continue;
    }

    writeInfo(tr("Generate stacks for file '%1'").arg(file.fileName()));

    QApplication::processEvents();

    generateStacks(m_dirs[file.fileName()]);

    progress += increment;
    m_progress->setValue(progress*100.0);
  }

  m_progress->setValue(100);
  writeImportant(tr("Process finished!"));
}

//----------------------------------------------------------------------
bool FakeStacks::isVersion5or6file(const QByteArray &fileInfo)
{
  return fileInfo.contains("SegFile Version=5") || fileInfo.contains("SegFile Version=6");
}

//----------------------------------------------------------------------
void FakeStacks::parseStacks(const QByteArray& data)
{
  QByteArray fileToken = "File=";

  auto begin = data.indexOf(fileToken, 0);
  while(begin != -1)
  {
    auto end     = data.indexOf(0x0A, begin);
    auto idStart = data.lastIndexOf('{', begin);
    auto idEnd   = data.lastIndexOf('}', begin);

    if(idStart != -1 && idEnd != -1)
    {
      auto fileState = QString::fromLatin1(data.mid(begin, end-begin));
      auto stateParts = fileState.split(';');
      stateParts = stateParts.first().split('=');

      auto filename = stateParts.at(1);
      QFileInfo file{filename};
      filename = file.fileName();

      if(!filename.contains("segmha", Qt::CaseInsensitive))
      {
        auto id = QString::fromLatin1(data.mid(idStart, idEnd-idStart+1));
        m_ids.insert(filename, id);
        writeInfo(tr("Found file '%1' with id '%2'").arg(filename).arg(id));
      }
    }

    begin = data.indexOf(fileToken, begin+1);
  }
}

//----------------------------------------------------------------------
void FakeStacks::parseBounds(QuaZip& zip)
{
  for(auto file: m_ids.keys())
  {
    auto name = tr("Filters/%1/outputs.xml").arg(m_ids[file]);

    QByteArray content;
    try
    {
      content = ZipUtils::readFileFromZip(name, zip);
    }
    catch(const EspinaException &e)
    {
      writeError(tr("ERROR: can't get stack xml information: '%1' (%2)").arg(file).arg(m_ids[file]));
      writeError(tr("CAUSE: %1").arg(e.details()));
      return;
    }

    QByteArray tokenBegin = "spacing=";
    QByteArray tokenEnd = "}\"";
    NmVector3 spacing;
    Bounds bounds;

    int begin = content.indexOf(tokenBegin, 0);

    if(begin != -1)
    {
      auto end = content.indexOf(tokenEnd, begin);
      if(end != -1)
      {
        spacing = parseSpacing(content.mid(begin+10, end-begin-10));
      }
      else
      {
        writeError(tr("ERROR: can't find end index while parsing spacing"));
        return;
      }
    }

    tokenBegin = QString("bounds=").toLatin1();

    begin = content.indexOf(tokenBegin, 0);
    if(begin != -1)
    {
      auto end = content.indexOf(tokenEnd, begin);

      if(end != -1)
      {
        if(spacing == NmVector3{0,0,0})
        {
          writeError(tr("ERROR: Spacing not parsed!"));
          return;
        }

        bounds = parseBounds(content.mid(begin+9, end-begin-9));
      }
      else
      {
        writeError(tr("ERROR: can't find end index while parsing bounds"));
        return;
      }
    }

    if(!bounds.areValid() || spacing == NmVector3())
    {
      writeError(tr("ERROR: parsing spacing (%1) or bounds (%2)").arg(spacing.toString()).arg(bounds.toString()));
      continue;
    }

    writeInfo(tr("Parsed spacing of stack '%1': %2").arg(file).arg(spacing.toString()));
    writeInfo(tr("Parsed bounds of stack '%1': %2").arg(file).arg(bounds.toString()));

    m_stacks.insert(file, bounds);
    m_spacing.insert(file, spacing);
  }
}

//----------------------------------------------------------------------
ESPINA::Bounds FakeStacks::parseBounds(const QByteArray& data)
{
  Bounds result;

  int begin = 0;
  for(int i = 0; i < 3; ++i)
  {
    auto begin1 = data.indexOf('(', begin);
    auto begin2 = data.indexOf('[', begin);

    if((begin1 != -1) && (begin2 != -1))
    {
      begin = std::min(begin1,begin2);
    }
    else
    {
      if(begin1 != -1)
      {
        begin = begin1;
      }
      else
      {
        if(begin2 != -1)
        {
          begin = begin2;
        }
        else
        {
          writeError("ERROR: parsing bounds begin, can't found index");
          return Bounds();
        }
      }
    }

    result.setLowerInclusion(toAxis(i), (data[begin] == '['));

    auto end1 = data.indexOf(')', begin);
    auto end2 = data.indexOf(']', begin);

    int end = 0;
    if((end1 != -1) && (end2 != -1))
    {
      end = std::min(end1, end2);
    }
    else
    {
      if(end1 != -1)
      {
        end = end1;
      }
      else
      {
        if(end2 != -1)
        {
          end = end2;
        }
        else
        {
          writeError("ERROR: parsing bounds end, can't found index");
          return Bounds();
        }
      }
    }

    result.setUpperInclusion(toAxis(i), (data[end]==']'));

    auto parts = QString::fromLatin1(data.mid(begin+1, end-begin-1)).split(',');

    result[2*i] = parts[0].toDouble();
    result[(2*i)+1] = parts[1].toDouble();
    begin = end;
  }

  return result;
}

//----------------------------------------------------------------------
ESPINA::NmVector3 FakeStacks::parseSpacing(const QByteArray& data, const QChar separator)
{
  auto numbers = QString::fromLatin1(data).split(separator);
  if(numbers.size() != 3)
  {
    writeError("Error parsing spacing, numbers != 3.");
    return NmVector3();
  }

  return NmVector3{numbers[0].toDouble(), numbers[1].toDouble(), numbers[2].toDouble()};
}

//----------------------------------------------------------------------
void FakeStacks::generateStacks(const QString &path)
{
  Bounds displacedBounds;

  if(m_displaced->isChecked())
  {
    auto bounds = m_stacks.begin().value();
    auto spacing = m_spacing.begin().value();

    displacedBounds = bounds;

    int size1 = (bounds[1]-bounds[0]) * 0.25;
    int size2 = (bounds[3]-bounds[2]) * 0.25;

    displacedBounds[0] += rand() % size1;
    displacedBounds[1] -= rand() % size1;
    displacedBounds[2] += rand() % size2;
    displacedBounds[3] -= rand() % size2;

    VolumeBounds corrected{displacedBounds, spacing, NmVector3()};
    displacedBounds = corrected.bounds();

    Q_ASSERT(contains(bounds, displacedBounds, spacing));
    Q_ASSERT(displacedBounds.areValid());
  }

  for(auto file: m_stacks.keys())
  {
    writeInfo(tr("Stack name: %1").arg(file));
    writeInfo(tr("Stack id: %1").arg(m_ids[file]));
    writeInfo(tr("Stack spacing: %1").arg(m_spacing[file].toString()));
    writeInfo(tr("Stack bounds: %1").arg(m_stacks[file].toString()));
    writeInfo(tr("Generating stack..."));

    QApplication::processEvents();

    auto spacing = m_spacing[file];
    auto bounds  = m_stacks[file];

    auto fileName = path + '/' + file.simplified();
    fileName = QDir::toNativeSeparators(fileName);

    auto region = equivalentRegion<ESPINA::itkVolumeType>(NmVector3{}, spacing, bounds);

    NmVector3 origin{region.GetIndex(0)*spacing[0], region.GetIndex(1)*spacing[1], region.GetIndex(2)*spacing[2]};

    itkVolumeType::Pointer image = itkVolumeType::New();
    image->SetSpacing(ItkSpacing<itkVolumeType>(spacing));
    image->SetOrigin(ItkPoint<itkVolumeType>(origin));
    image->SetRegions(region);
    image->Allocate();
    image->Update();

    std::memset(image->GetBufferPointer(), 0, region.GetSize(0)*region.GetSize(1)*region.GetSize(2));

    if(!displacedBounds.areValid())
    {
      displacedBounds = bounds;
    }

    auto iterator = itkImageIteratorWithIndex<itkVolumeType>(image, displacedBounds);
    itkVolumeType::IndexType index;

    while(!iterator.IsAtEnd())
    {
      index = iterator.GetIndex();
      int value = static_cast<int>(255 * noise(index.GetElement(0),index.GetElement(1),index.GetElement(2))) % 255;
      iterator.Set(value);

      ++iterator;
    }

    auto writer = itk::ImageFileWriter<itkVolumeType>::New();
    writer->SetFileName(fileName.toStdString());
    writer->SetInput(image);
    writer->SetNumberOfThreads(1);
    writer->SetUseCompression(false);
    writer->Write();
  }
}

//----------------------------------------------------------------------
double FakeStacks::noise(unsigned int x, unsigned int y, unsigned int z)
{
   int n = (x + (31337 * y) + (263 * z)  + 10130) & 0x7fffffff;
   n = (n >> 13) ^ n;
   n = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;

   return 1.0 - (n / 1073741824.0);
}
