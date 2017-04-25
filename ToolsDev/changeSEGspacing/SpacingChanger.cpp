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
#include <Core/Analysis/Extensions.h>
#include <Core/Analysis/Persistent.h>
#include <Core/IO/ZipUtils.h>
#include <Core/Utils/EspinaException.h>
#include <Core/Utils/VolumeBounds.h>
#include <Core/Utils/vtkPolyDataUtils.h>

// Qt
#include <QAbstractButton>
#include <QApplication>

// Quazip
#include <quazip/quazip.h>
#include <ToolsDev/changeSEGspacing/SpacingChanger.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyData.h>
#include <vtkCommunicator.h>
#include <vtkGenericDataObjectReader.h>

using namespace ESPINA;
using namespace ESPINA::IO;
using namespace ESPINA::Core::Utils;

const QString VERSION_FILE        = "formatInfo.ini";
const QString CONTENT_FILE        = "content.dot";
const QString RELATIONS_FILE      = "relations.dot";

//----------------------------------------------------------------------
SpacingChanger::SpacingChanger(QWidget *parent, Qt::WindowFlags flags)
: QDialog{parent, flags}
{
  setupUi(this);

  setWindowTitle(tr("Change SEG files spacing"));
  setWindowIcon(QIcon(":/Tools/espina.svg"));

  m_progress->setEnabled(false);
  m_progress->setMaximum(100);
  m_progress->setMinimum(0);
  m_progress->setValue(0);

  m_xSpacing->setValue(1.0);
  m_ySpacing->setValue(1.0);
  m_zSpacing->setValue(1.0);

  connect(m_addButton, SIGNAL(clicked()), this, SLOT(addFiles()));
  connect(m_quit, SIGNAL(clicked()), this, SLOT(close()));
  connect(m_start, SIGNAL(clicked()), this, SLOT(startConversion()));

  m_start->setEnabled(false);

  setFixedWidth(1000);
}

//----------------------------------------------------------------------
SpacingChanger::~SpacingChanger()
{
}

//----------------------------------------------------------------------
void SpacingChanger::startConversion()
{
  m_errors->setText("0");
  m_converted->setText("0");

  m_progress->setEnabled(true);
  m_progress->setValue(0);

  double progress = 0;
  NmVector3 spacing{0,0,0};

  for(auto file: m_files)
  {
    QApplication::processEvents();

    QFileInfo fileInfo{file};
    if(!fileInfo.exists())
    {
      writeError(tr("ERROR: %1 doesn't exists").arg(fileInfo.absoluteFilePath()));
      increaseErrors();
      continue;
    }

    QuaZip sourceZip{fileInfo.absoluteFilePath()};
    sourceZip.open(QuaZip::mdUnzip, nullptr);

    if(sourceZip.getZipError() != UNZ_OK)
    {
      writeError(tr("ERROR: couldn't open file: %1. Cause: %2").arg(fileInfo.fileName()).arg(sourceZip.getZipError()));
      progress += static_cast<double>(1/m_files.size())*100;
      m_progress->setValue(progress);
      increaseErrors();
      continue;
    }

    try
    {
      if(!isVersion6file(ZipUtils::readFileFromZip(VERSION_FILE, sourceZip)))
      {
        writeError(tr("ERROR: file '%1' is not SEG file version 6.").arg(file.fileName()));
        increaseErrors();
        continue;
      }
    }
    catch(const EspinaException &e)
    {
      writeError(tr("ERROR: Error uncompressing file: %1").arg(VERSION_FILE));
      writeError(tr("CAUSE: %1").arg(e.details()));
      increaseErrors();
      continue;
    }

    auto otherFileName = fileInfo.path() +
                         '/' +
                         fileInfo.baseName() +
                         tr("_(scaled_%1_%2_%3).").arg(m_xSpacing->value()).arg(m_ySpacing->value()).arg(m_zSpacing->value())
                         + fileInfo.suffix();
    otherFileName = QDir::toNativeSeparators(otherFileName);

    writeImportant(tr("Processing '%1' to '%2'.").arg(fileInfo.fileName()).arg(otherFileName));

    QuaZip destZip(QDir::toNativeSeparators(otherFileName));
    destZip.open(QuaZip::mdCreate, nullptr);

    if(destZip.getZipError() != UNZ_OK)
    {
      writeError(tr("ERROR: couldn't create destination file: %1. Error: %2").arg(otherFileName).arg(destZip.getZipError()));
      progress += static_cast<double>(1/m_files.size())*100;
      m_progress->setValue(progress);
      increaseErrors();
      continue;
    }

    try
    {
      QByteArray contents = ZipUtils::readFileFromZip(CONTENT_FILE, sourceZip);
      m_spacing = getSpacing(contents);

      writeImportant(tr("File spacing: %1").arg(m_spacing.toString()));
    }
    catch(const EspinaException &e)
    {
      writeError(tr("ERROR: Error uncompressing file: %1").arg(CONTENT_FILE));
      writeError(tr("CAUSE: %1").arg(e.details()));
      increaseErrors();
      continue;
    }

    auto increment = 1/(sourceZip.getFileNameList().size()*static_cast<double>(m_files.size()));

    for(bool f = sourceZip.goToFirstFile(); f; f = sourceZip.goToNextFile())
    {
      auto zFileName = sourceZip.getCurrentFileName();
      QByteArray contents;

      try
      {
        contents = ZipUtils::readFileFromZip(sourceZip.getCurrentFileName(), sourceZip);
      }
      catch(const EspinaException &e)
      {
        writeError(tr("ERROR: Error uncompressing file: %1").arg(zFileName));
        writeError(tr("CAUSE: %1").arg(e.details()));
        increaseErrors();
        continue;
      }

      writeImportant(tr("Processing File: %1").arg(zFileName));
      QApplication::processEvents();
      if(zFileName.endsWith(".xml", Qt::CaseInsensitive))
      {
        if(zFileName.startsWith("Extensions/"))
        {
          purgeInfo(contents);
        }

        processXML(contents);
      }

      if((zFileName == CONTENT_FILE) || (zFileName == RELATIONS_FILE))
      {
        processGraph(contents);
      }

      if(zFileName.endsWith(".mhd", Qt::CaseInsensitive))
      {
        processMHD(contents);
      }

      if(zFileName.endsWith(".vtp", Qt::CaseInsensitive))
      {
        contents = processMesh(contents, m_spacing);
      }

      if(zFileName.endsWith(".vti", Qt::CaseInsensitive))
      {
        processStencil(contents);
      }

      if(zFileName.endsWith(".bin", Qt::CaseInsensitive) && zFileName.contains("roi", Qt::CaseInsensitive))
      {
        contents = processROI(contents);
      }

      if(zFileName.endsWith(".txt", Qt::CaseInsensitive) && zFileName.startsWith("Extra"))
      {
        writeInfo(tr("Not archiving extra file: '%1'").arg(zFileName));
        continue;
      }

      if(!zFileName.endsWith(".txt", Qt::CaseInsensitive) && !zFileName.endsWith(".mhd", Qt::CaseInsensitive) &&
         !zFileName.endsWith(".dot", Qt::CaseInsensitive) && !zFileName.endsWith(".raw", Qt::CaseInsensitive) &&
         !zFileName.endsWith(".xml", Qt::CaseInsensitive) && !zFileName.endsWith(".bin", Qt::CaseInsensitive) &&
         !zFileName.endsWith(".ini", Qt::CaseInsensitive) && !zFileName.endsWith(".vtp", Qt::CaseInsensitive) &&
         !zFileName.endsWith(".vti", Qt::CaseInsensitive))
      {
        increaseErrors();
        writeError(tr("ERROR: unknown file '%1'").arg(zFileName));
      }

      writeInfo(tr("Compress %1").arg(zFileName));

      // FIX wrong adaptive edges path.
      if(zFileName.contains("AdaptiveEdges"))
      {
        auto parts = zFileName.split('/');
        zFileName = tr("Extensions/AdaptiveEdges/%1").arg(parts.last());
      }

      try
      {
        ZipUtils::AddFileToZip(zFileName, contents, destZip);
      }
      catch(const EspinaException &e)
      {
        writeError(tr("ERROR: Error compressing file: %1").arg(zFileName));
        increaseErrors();
      }

      increaseConverted();
      progress += increment;
      m_progress->setValue(progress*100);
      QApplication::processEvents();
    }

    sourceZip.close();
    destZip.close();
  }

  m_progress->setEnabled(false);
  m_progress->setValue(100);
}

//----------------------------------------------------------------------
void SpacingChanger::addFiles()
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

        writeImportant(tr("Added file: %1").arg(fileInfo.fileName()));

        m_start->setEnabled(true);
      }
    }
  }
}

//----------------------------------------------------------------------
bool SpacingChanger::isVersion6file(const QByteArray &fileInfo)
{
  return fileInfo.contains("SegFile Version=6");
}

//----------------------------------------------------------------------
void SpacingChanger::processXML(QByteArray& data)
{
  QByteArray tokenBegin = "Spacing=";
  QByteArray tokenEnd   = ";";
  QByteArray replacement = QString("%1,%2,%3").arg(m_xSpacing->value()).arg(m_ySpacing->value()).arg(m_zSpacing->value()).remove(' ').toAscii();
  NmVector3 spacing;

  int begin = 0;
  while(begin != -1)
  {
    begin = data.indexOf(tokenBegin, begin);

    if(begin != -1)
    {
      auto end = data.indexOf(tokenEnd, begin);

      if(end != -1)
      {
        spacing = parseSpacing(data.mid(begin+tokenBegin.length(), end-begin-tokenBegin.length()));
        writeInfo(tr("Parsed spacing: %1").arg(spacing.toString()));

        writeInfo(tr("Replaced '%1' with '%2'").arg(QString::fromAscii(data.mid(begin+tokenBegin.length(), end-begin-tokenBegin.length())))
                                               .arg(QString::fromAscii(replacement)));

        data.remove(begin+tokenBegin.length(), end-begin-tokenBegin.length());
        data.insert(begin+tokenBegin.length(), replacement);

        begin += replacement.length() + tokenBegin.length();
      }
      else
      {
        writeError(tr("ERROR: can't find end index while parsing spacing"));
        increaseErrors();
        return;
      }
    }
  }

  tokenBegin = "spacing=";
  tokenEnd = "}\"";

  begin = 0;
  while(begin != -1)
  {
    begin = data.indexOf(tokenBegin, begin);

    if(begin != -1)
    {
      auto end = data.indexOf(tokenEnd, begin);

      if(end != -1)
      {
        spacing = parseSpacing(data.mid(begin+10, end-begin-10));
        writeInfo(tr("Parsed spacing: %1").arg(spacing.toString()));

        writeInfo(tr("Replaced '%1' with '%2'").arg(QString::fromAscii(data.mid(begin+10, end-begin-10)))
                                               .arg(QString::fromAscii(replacement)));

        data.remove(begin+10, end-begin-10);
        data.insert(begin+10, replacement);

        begin += 10 + replacement.length();
      }
      else
      {
        writeError(tr("ERROR: can't find end index while parsing spacing"));
        increaseErrors();
        return;
      }
    }
  }

  tokenBegin = QString("bounds=").toAscii();

  begin = 0;
  while(begin != -1)
  {
    begin = data.indexOf(tokenBegin, begin);

    if(begin != -1)
    {
      auto end = data.indexOf(tokenEnd, begin);

      if(end != -1)
      {
        if(spacing == NmVector3{0,0,0})
        {
          writeError(tr("ERROR: Spacing not parsed!"));
          return;
        }

        auto bounds = parseBounds(data.mid(begin+9, end-begin-9));
        bounds[0] = bounds[0]/spacing[0] * m_xSpacing->value();
        bounds[1] = bounds[1]/spacing[0] * m_xSpacing->value();
        bounds[2] = bounds[2]/spacing[1] * m_ySpacing->value();
        bounds[3] = bounds[3]/spacing[1] * m_ySpacing->value();
        bounds[4] = bounds[4]/spacing[2] * m_zSpacing->value();
        bounds[5] = bounds[5]/spacing[2] * m_zSpacing->value();

        replacement = bounds.toString().remove(' ').remove('{').remove('}').toAscii();

        writeInfo(tr("Replaced '%1' with '%2'").arg(QString::fromAscii(data.mid(begin+9, end-begin-9)))
                                               .arg(QString::fromAscii(replacement)));

        data.remove(begin+9, end-begin-9);
        data.insert(begin+9, replacement);

        begin += 9 + replacement.length();
      }
      else
      {
        writeError(tr("ERROR: can't find end index while parsing bounds"));
        increaseErrors();
        return;
      }
    }
  }

  // Counting frame
  tokenBegin = QString("<Extension Type=\"CountingFrame\"").toAscii();

  begin = 0;
  while(begin != -1)
  {
    begin = data.indexOf(tokenBegin, begin);

    if(begin != -1)
    {
      tokenBegin = QString("<State>").toAscii();
      tokenEnd   = QString("</State>").toAscii();

      auto cfbegin = data.indexOf(tokenBegin, begin);
      auto cfend = data.indexOf(tokenEnd, begin);

      if(cfbegin != -1 && cfend != cfbegin+tokenBegin.length())
      {
        auto total = data.mid(cfbegin+tokenBegin.length(), cfend-cfbegin-tokenBegin.length());
        auto CFlist = total.split('\n');
        QByteArray replacement;

        for(auto CF: CFlist)
        {
          auto parts = CF.split(',');
          if(parts.size() != 9)
          {
            writeError(tr("ERROR: Invalid Counting Frame parts number: %1 instead of 9.").arg(parts.size()));
            increaseErrors();

            continue;
          }

          NmVector3 inclusion, exclusion;
          inclusion[0] = parts[3].toDouble();
          inclusion[1] = parts[4].toDouble();
          inclusion[2] = parts[5].toDouble();
          exclusion[0] = parts[6].toDouble();
          exclusion[1] = parts[7].toDouble();
          exclusion[2] = parts[8].toDouble();

          inclusion[0] = inclusion[0]/m_spacing[0] * m_xSpacing->value();
          inclusion[1] = inclusion[1]/m_spacing[1] * m_ySpacing->value();
          inclusion[2] = inclusion[2]/m_spacing[2] * m_zSpacing->value();
          exclusion[0] = exclusion[0]/m_spacing[0] * m_xSpacing->value();
          exclusion[1] = exclusion[1]/m_spacing[1] * m_ySpacing->value();
          exclusion[2] = exclusion[2]/m_spacing[2] * m_zSpacing->value();

          auto CFreplacement = QString("%1,%2,%3,%4,%5,%6,%7,%8,%9").arg(QString(parts[0]))
                                                             .arg(QString(parts[1]))
                                                             .arg(QString(parts[2]))
                                                             .arg(QString::number(inclusion[0]))
                                                             .arg(QString::number(inclusion[1]))
                                                             .arg(QString::number(inclusion[2]))
                                                             .arg(QString::number(exclusion[0]))
                                                             .arg(QString::number(exclusion[1]))
                                                             .arg(QString::number(exclusion[2]));


          writeInfo(tr("Counting Frame: Replaced '%1' with '%2'").arg(QString::fromAscii(CF))
                                                     .arg(CFreplacement));

          replacement.append(CFreplacement.toAscii());
          if(CF != CFlist.last()) replacement.append('\n');
        }

        data.remove(cfbegin+tokenBegin.length(), total.size());
        data.insert(cfbegin+tokenBegin.length(), replacement);
      }
      else
      {
        writeError(tr("ERROR: Empty Counting frame extension list."));
        increaseErrors();
        return;
      }

      ++begin;
    }
  }
}

//----------------------------------------------------------------------
void SpacingChanger::processGraph(QByteArray& data)
{
  QByteArray replacement = QString("%1,%2,%3").arg(m_xSpacing->value()).arg(m_ySpacing->value()).arg(m_zSpacing->value()).toAscii();
  QByteArray tokenBegin = "Spacing=";
  QByteArray tokenEnd   = ";";
  NmVector3 spacing, seed;

  int begin = 0;
  while(begin != -1)
  {
    begin = data.indexOf(tokenBegin, begin);

    if(begin != -1)
    {
      auto end = data.indexOf(tokenEnd, begin);

      if(end != -1)
      {
        spacing = parseSpacing(data.mid(begin+8, end-begin-8));
        writeInfo(tr("Parsed spacing: %1").arg(spacing.toString()));
        writeInfo(tr("Replaced '%1' with '%2'").arg(QString::fromAscii(data.mid(begin+8, end-begin-8)))
                                               .arg(QString::fromAscii(replacement)));

        data.remove(begin+8, end-begin-8);
        data.insert(begin+8, replacement);

        begin += 8 + replacement.length();
      }
    }
  }

  tokenBegin = "Seed=";

  begin = 0;
  while(begin != -1)
  {
    begin = data.indexOf(tokenBegin, begin);

    if(begin != -1)
    {
      auto end = data.indexOf(tokenEnd, begin);

      if(end != -1)
      {
        if(spacing == NmVector3{0,0,0})
        {
          writeError(tr("ERROR: Spacing not parsed!"));
          return;
        }

        seed = parseSpacing(data.mid(begin+5, end-begin-5));
        writeInfo(tr("Parsed seed: %1").arg(seed.toString()));

        seed[0] = seed[0]/spacing[0] * m_xSpacing->value();
        seed[1] = seed[1]/spacing[1] * m_ySpacing->value();
        seed[2] = seed[2]/spacing[2] * m_zSpacing->value();

        replacement = QString("%1,%2,%3").arg(seed[0]).arg(seed[1]).arg(seed[2]).toAscii();

        writeInfo(tr("Replaced '%1' with '%2'").arg(QString::fromAscii(data.mid(begin+5, end-begin-5)))
                                               .arg(QString::fromAscii(replacement)));

        data.remove(begin+8, end-begin-8);
        data.insert(begin+8, replacement);

        begin += 8 + replacement.length();
      }
    }
  }
}

//----------------------------------------------------------------------
void SpacingChanger::processMHD(QByteArray &data)
{
  QByteArray replacement = QString("%1 %2 %3").arg(m_xSpacing->value()).arg(m_ySpacing->value()).arg(m_zSpacing->value()).toAscii();
  QByteArray tokenBegin = "Spacing = ";
  NmVector3 spacing;

  int begin = 0;
  while(begin != -1)
  {
    begin = data.indexOf(tokenBegin, begin);

    if(begin != -1)
    {
      auto end = data.indexOf(0x0A, begin);

      if(end != -1)
      {
        spacing = parseSpacing(data.mid(begin+tokenBegin.length(), end-begin-tokenBegin.length()), ' ');
        writeInfo(tr("Parsed spacing: %1").arg(spacing.toString()));
        writeInfo(tr("Replaced '%1' with '%2'").arg(QString::fromAscii(data.mid(begin+tokenBegin.length(), end-begin-tokenBegin.length())))
                                               .arg(QString::fromAscii(replacement)));

        data.remove(begin+tokenBegin.length(), end-begin-tokenBegin.length());
        data.insert(begin+tokenBegin.length(), replacement);

        begin += tokenBegin.length() + replacement.length();
      }
    }
  }

  tokenBegin = "Offset = ";

  begin = 0;
  while(begin != -1)
  {
    begin = data.indexOf(tokenBegin, begin);

    if(begin != -1)
    {
      auto end = data.indexOf(0x0A, begin);

      if(end != -1)
      {
        NmVector3 offset = parseSpacing(data.mid(begin+tokenBegin.length(), end-begin-tokenBegin.length()), ' ');
        writeInfo(tr("Parsed offset: %1").arg(offset.toString()));

        offset[0] = offset[0]/spacing[0] * m_xSpacing->value();
        offset[1] = offset[1]/spacing[1] * m_ySpacing->value();
        offset[2] = offset[2]/spacing[2] * m_zSpacing->value();

        replacement = QString("%1 %2 %3").arg(offset[0]).arg(offset[1]).arg(offset[2]).toAscii();

        writeInfo(tr("Replaced '%1' with '%2'").arg(QString::fromAscii(data.mid(begin+tokenBegin.length(), end-begin-tokenBegin.length())))
                                               .arg(QString::fromAscii(replacement)));

        data.remove(begin+tokenBegin.length(), end-begin-tokenBegin.length());
        data.insert(begin+tokenBegin.length(), replacement);

        begin += tokenBegin.length() + replacement.length();
      }
    }
  }
}

//----------------------------------------------------------------------
QByteArray SpacingChanger::processROI(const QByteArray &data)
{
  auto original = deserializeVolumeBounds(data);
  Bounds converted, oldBounds = original.bounds();
  auto spacing = original.spacing();

  converted[0] = oldBounds[0]/spacing[0] * m_xSpacing->value();
  converted[1] = oldBounds[1]/spacing[0] * m_xSpacing->value();
  converted[2] = oldBounds[2]/spacing[1] * m_ySpacing->value();
  converted[3] = oldBounds[3]/spacing[1] * m_ySpacing->value();
  converted[4] = oldBounds[4]/spacing[2] * m_zSpacing->value();
  converted[5] = oldBounds[5]/spacing[2] * m_zSpacing->value();

  VolumeBounds vBounds{converted,
                       NmVector3{m_xSpacing->value(), m_ySpacing->value(), m_zSpacing->value()},
                       original.origin()/original.spacing() * NmVector3{m_xSpacing->value(), m_ySpacing->value(), m_zSpacing->value()}};

  writeInfo(tr("Converted '%1' in '%2'").arg(original.toString()).arg(vBounds.toString()));

  return serializeVolumeBounds(vBounds);
}

//----------------------------------------------------------------------
void SpacingChanger::purgeInfo(QByteArray& data)
{
  QByteArray extBegin = "<Extension Type=";
  QByteArray invToken = "InvalidateOnChange=\"1\"";

  int begin = 0;
  while(begin != -1)
  {
    begin = data.indexOf(extBegin, begin);

    if(begin != -1)
    {
      auto extEnd = data.indexOf(extBegin, begin+1);
      auto invalidate = data.indexOf(invToken, begin);

      if(invalidate != -1 && ((invalidate < extEnd) || (extEnd == -1)))
      {
        writeInfo(tr("Purging info of extension '%1'.").arg(QString::fromAscii(data.mid(begin+1, invalidate-begin-2))));
        QByteArray infoBegin{"<Info Name="};
        QByteArray infoEnd{"Info>"};

        int iBegin = data.indexOf(infoBegin, invalidate);
        while(iBegin != -1 && ((iBegin < extEnd) || (extEnd == -1)))
        {
          auto iEnd = data.indexOf(infoEnd, iBegin);

          if(iEnd != -1)
          {
            writeInfo(tr("Purged: %1").arg(QString::fromAscii(data.mid(iBegin, iEnd+infoEnd.length()-iBegin))));
            data.remove(iBegin, iEnd+infoEnd.length()-iBegin);
            extEnd = data.indexOf(extBegin, begin+1);
          }
          else
          {
            writeError(tr("ERROR: can't find information end index"));
            increaseErrors();
            return;
          }

          iBegin = data.indexOf(infoBegin, iBegin);
        }
      }
      ++begin;
    }
  }
}

//----------------------------------------------------------------------
ESPINA::Bounds SpacingChanger::parseBounds(const QByteArray& data)
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
          increaseErrors();
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
          increaseErrors();
          return Bounds();
        }
      }
    }

    result.setUpperInclusion(toAxis(i), (data[end]==']'));

    auto parts = QString::fromAscii(data.mid(begin+1, end-begin-1)).split(',');

    result[2*i] = parts[0].toDouble();
    result[(2*i)+1] = parts[1].toDouble();
    begin = end;
  }

  return result;
}

//----------------------------------------------------------------------
ESPINA::NmVector3 SpacingChanger::parseSpacing(const QByteArray& data, const QChar separator)
{
  auto numbers = QString::fromAscii(data).split(separator);
  if(numbers.size() != 3)
  {
    writeError("Error parsing spacing, numbers != 3.");
    increaseErrors();
    return NmVector3();
  }

  return NmVector3{numbers[0].toDouble(), numbers[1].toDouble(), numbers[2].toDouble()};
}

//----------------------------------------------------------------------
ESPINA::NmVector3 SpacingChanger::getSpacing(const QByteArray &data)
{
  const QByteArray tokenBegin = "Spacing=";
  const QByteArray tokenEnd = ";";
  int begin = 0;
  NmVector3 spacing;

  if(begin != -1)
  {
    begin = data.indexOf(tokenBegin, begin);

    if(begin != -1)
    {
      auto end = data.indexOf(tokenEnd, begin);

      if(end != -1)
      {
        auto part = data.mid(begin+tokenBegin.size(), end-begin-tokenBegin.size());

        if(spacing == NmVector3())
        {
          spacing = parseSpacing(part);
        }
        else
        {
          auto otherSpacing = parseSpacing(part);

          if(otherSpacing != spacing)
          {
            writeError(tr("Different spacing in content.dot: %1 and %2").arg(spacing.toString()).arg(otherSpacing.toString()));
          }
        }
      }
      else
      {
        increaseErrors();
        writeError(tr("ERROR: getSpacing() can't find end index!"));
        throw EspinaException(tr("ERROR: can't find end index!"), tr("ERROR: can't find end index!"));
      }

      begin = end;
    }
  }

  if(spacing == NmVector3())
  {
    increaseErrors();
    writeError(tr("ERROR: getSpacing() No spacing in content.dot!"));
    throw EspinaException(tr("ERROR: No spacing in content.dot!"), tr("ERROR: No spacing in content.dot!"));
  }

  return spacing;
}

//----------------------------------------------------------------------
QByteArray SpacingChanger::processMesh(const QByteArray &data, const NmVector3 &spacing)
{
  if(spacing[0] == 0 || spacing[1] == 0 || spacing[2] == 0)
  {
    increaseErrors();
    writeError(tr("ERROR: processMesh() Null spacing when scaling mesh!"));
    throw EspinaException(tr("ERROR: Null spacing when scaling mesh!"), tr("ERROR: Null spacing when scaling mesh!"));
    return QByteArray();
  }

  NmVector3 ratio{m_xSpacing->value()/spacing[0], m_ySpacing->value()/spacing[1], m_zSpacing->value()/spacing[2]};

  auto reader = vtkSmartPointer<vtkGenericDataObjectReader>::New();
  reader->SetReadFromInputString(true);
  reader->SetReadAllFields(true);
  reader->SetInputString(data.constData(), data.length());
  reader->Update();

  auto polydata = vtkPolyData::SafeDownCast(reader->GetOutput());
  polydata->DeepCopy(reader->GetOutput());

  writeInfo(tr("Scaling mesh with %1 points.").arg(polydata->GetNumberOfPoints()));

  PolyDataUtils::scalePolyData(polydata, ratio);

  return ESPINA::PolyDataUtils::savePolyDataToBuffer(polydata);
}

//----------------------------------------------------------------------
void SpacingChanger::processStencil(QByteArray &data)
{
  QByteArray spacingToken = "SPACING ";
  QByteArray originToken  = "ORIGIN ";
  auto endToken = 0x0A;
  NmVector3 spacing;

  int begin = data.indexOf(spacingToken,0);

  if(begin != -1)
  {
    auto end = data.indexOf(endToken, begin);

    spacing = parseSpacing(data.mid(begin+spacingToken.length(), end-begin-spacingToken.length()), ' ');

    auto replacement = tr("%1 %2 %3").arg(m_xSpacing->value()).arg(m_ySpacing->value()).arg(m_zSpacing->value());
    data.remove(begin+spacingToken.length(), end-begin-spacingToken.length());
    data.insert(begin+spacingToken.length(), replacement.toAscii());

    writeInfo(tr("Replaced spacing '%1' with '%2'.").arg(spacing.toString()).arg(replacement));
  }

  begin = data.indexOf(originToken, 0);

  if(begin != -1)
  {
    auto end = data.indexOf(endToken, begin);

    auto origin = parseSpacing(data.mid(begin+originToken.length(), end-begin-originToken.length()), ' ');
    if(areEqual(origin[0], 0, spacing[0])) origin[0] = 0;
    if(areEqual(origin[1], 0, spacing[1])) origin[1] = 0;
    if(areEqual(origin[2], 0, spacing[2])) origin[2] = 0;

    NmVector3 point{origin[0]/spacing[0] * m_xSpacing->value(), origin[1]/spacing[1] * m_ySpacing->value(), origin[2]/spacing[2] * m_zSpacing->value()};

    auto replacement = tr("%1 %2 %3").arg(point[0]).arg(point[1]).arg(point[2]);

    data.remove(begin+spacingToken.length(), end-begin-spacingToken.length());
    data.insert(begin+spacingToken.length(), replacement.toAscii());

    writeInfo(tr("Replaced origin '%1' with '%2'.").arg(origin.toString()).arg(replacement));
  }
}
