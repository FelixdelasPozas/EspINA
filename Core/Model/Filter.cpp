/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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

// EspINA
#include "Filter.h"
#include "EspinaModel.h"
#include "Output.h"
#include "Core/Outputs/VolumeRepresentation.h"

// ITK
#include <itkMetaImageIO.h>

// Qt
#include <QDir>
#include <QMessageBox>
#include <QWidget>
#include <QDebug>

using namespace EspINA;

const QString EspINA::Filter::CREATELINK = "CreateSegmentation";

typedef ModelItem::ArgumentId ArgumentId;

const ArgumentId Filter::ID     = "ID";
const ArgumentId Filter::INPUTS = "Inputs";
const ArgumentId Filter::EDIT   = "Edit"; // Backwards compatibility


//----------------------------------------------------------------------------
Filter::~Filter()
{
//   qDebug() << "Destroying Filter";
}

//----------------------------------------------------------------------------
void Filter::setCacheDir(QDir dir)
{
  m_cacheDir = dir;

}

//----------------------------------------------------------------------------
Filter::Filter(Filter::NamedInputs  namedInputs,
               ModelItem::Arguments args,
               FilterType           type)
: m_namedInputs(namedInputs)
, m_args(args)
, m_type(type)
, m_cacheId(-1)
, m_traceable(false)
, m_executed(false)
{
  if (m_args.contains(ID)) {
    m_cacheId = m_args[ID].toInt();
  } else {
    m_args[ID] = "-1";
  }
}

//----------------------------------------------------------------------------
QVariant Filter::data(int role) const
{
  if (Qt::DisplayRole == role)
    return m_type;
  else
    return QVariant();
}

//----------------------------------------------------------------------------
QString Filter::serialize() const
{
  // NOTE: EDIT arg is being deprecated
  Arguments copy = m_args;
  copy.remove(EDIT);

  return copy.serialize();
}

//----------------------------------------------------------------------------
bool SegmentationFilter::needUpdate() const
{
  bool update = true;

  if (!m_outputs.isEmpty())
  {
    update = false;
    foreach(SegmentationOutputSPtr filterOutput, m_outputs)
    {
      update = update || !filterOutput->isValid();
    }
  }

  return update;
}

//----------------------------------------------------------------------------
bool SegmentationFilter::needUpdate(FilterOutputId oId) const
{
  return !m_outputs.contains(oId) || !m_outputs[oId]->isValid();
}

//----------------------------------------------------------------------------
void Filter::update()
{
  if (numberOfOutputs() == 0)
  {
    m_inputs.clear();

    QStringList namedInputList = m_args[INPUTS].split(",", QString::SkipEmptyParts);
    foreach(QString namedInput, namedInputList)
    {
      QStringList input = namedInput.split("_");
      FilterSPtr inputFilter = m_namedInputs[input[0]];
      FilterOutputId iId = input[1].toInt();
      inputFilter->update(iId);
      m_inputs << inputFilter->output(iId);
    }

    run();

    m_executed = true;
  }
  else
  {
    foreach(FilterOutputId oId, availableOutputIds())
    {
      update(oId);
    }
  }
}

//----------------------------------------------------------------------------
void SegmentationFilter::update(FilterOutputId oId)
{
  bool ignoreOutputs     = ignoreCurrentOutputs();
  bool outputNeedsUpdate = needUpdate(oId);

   if (ignoreOutputs || outputNeedsUpdate)
   {
     // Invalidate previous edited regions
     if (ignoreOutputs && m_outputs.contains(oId))
     {
       m_outputs[oId]->clearEditedRegions();
     }

     if (!fetchSnapshot(oId))
     {
       m_inputs.clear();

       QStringList namedInputList = m_args[INPUTS].split(",", QString::SkipEmptyParts);
       foreach(QString namedInput, namedInputList)
       {
         QStringList input = namedInput.split("_");
         FilterSPtr inputFilter = m_namedInputs[input[0]];
         FilterOutputId iId = input[1].toInt();
         inputFilter->update(iId);
         m_inputs << inputFilter->output(iId);
       }

       run(oId);

       if (m_outputs.contains(oId))
       {
         m_outputs[oId]->restoreEditedRegions(m_cacheDir, cacheOutputId(oId));
       }

       m_executed = true;
     }
   }
}

//----------------------------------------------------------------------------
bool SegmentationFilter::fetchSnapshot(FilterOutputId oId)
{
  return false;
}

//----------------------------------------------------------------------------
itkVolumeType::Pointer Filter::readVolumeFromCache(const QString &file)
{
  itkVolumeType::Pointer volume;

  if (m_cacheDir.exists(file))
  {
    itk::MetaImageIO::Pointer io = itk::MetaImageIO::New();
    EspinaVolumeReader::Pointer reader = EspinaVolumeReader::New();

    QByteArray tmpFile = m_cacheDir.absoluteFilePath(file).toUtf8();
    io->SetFileName(tmpFile);
    reader->SetImageIO(io);
    reader->SetFileName(tmpFile.data());
    reader->Update();

    volume = reader->GetOutput();
    volume->DisconnectPipeline();
  }

  return volume;
}


//----------------------------------------------------------------------------
void ChannelFilter::update(FilterOutputId oId)
{
  bool ignoreOutputs     = ignoreCurrentOutputs();
  bool outputNeedsUpdate = needUpdate(oId);

   if (ignoreOutputs || outputNeedsUpdate)
   {
     if (!fetchSnapshot(oId))
     {
       m_inputs.clear();

       QStringList namedInputList = m_args[INPUTS].split(",", QString::SkipEmptyParts);
       foreach(QString namedInput, namedInputList)
       {
         QStringList input = namedInput.split("_");
         FilterSPtr inputFilter = m_namedInputs[input[0]];
         FilterOutputId iId = input[1].toInt();
         inputFilter->update(iId);
         m_inputs << inputFilter->output(iId);
       }

       run(oId);

       m_executed = true;
     }
   }
}

//----------------------------------------------------------------------------
bool ChannelFilter::validOutput(FilterOutputId oId)
{
  return m_outputs.contains(oId);
}


//----------------------------------------------------------------------------
bool SegmentationFilter::validOutput(FilterOutputId oId)
{
  return m_outputs.contains(oId);
}

//----------------------------------------------------------------------------
bool SegmentationFilter::dumpSnapshot(Snapshot &snapshot)
{
  bool result = false;

  foreach(SegmentationOutputSPtr filterOutput, m_outputs)
  {
    FilterOutputId oId = filterOutput->id();

    // Backwards compatibility with seg files version 1.0
    if (m_model->isTraceable() && m_args.contains(EDIT))
    {
      qWarning("Deprecated Format");
      update(oId);

      QStringList outputIds = m_args[EDIT].split(",");
      if (outputIds.contains(QString::number(oId)))
      {
        SegmentationVolumeSPtr editedVolume = segmentationVolume(filterOutput);
        EspinaRegion region = editedVolume->espinaRegion();
        editedVolume->addEditedRegion(region);
      }
    }

    bool saveEditedRegions = m_model->isTraceable() && filterOutput->isEdited();

    if (filterOutput->isCached() || saveEditedRegions)
      result |= filterOutput->dumpSnapshot(cacheOutputId(oId), snapshot, saveEditedRegions);

    filterOutput->setCached(false);
  }

  return result;
}

//----------------------------------------------------------------------------
bool ChannelFilter::needUpdate() const
{
  bool update = true;

  if (!m_outputs.isEmpty())
  {
    update = false;
    foreach(ChannelOutputSPtr filterOutput, m_outputs)
    {
      update = update || !filterOutput->isValid();
    }
  }

  return update;
}

//----------------------------------------------------------------------------
bool ChannelFilter::needUpdate(FilterOutputId oId) const
{
  return !m_outputs.contains(oId) || !m_outputs[oId]->isValid();
}


//----------------------------------------------------------------------------
FilterPtr EspINA::filterPtr(ModelItemPtr item)
{
  Q_ASSERT(EspINA::FILTER == item->type());
  FilterPtr ptr = dynamic_cast<FilterPtr>(item);
  Q_ASSERT(ptr);

  return ptr;
}

//----------------------------------------------------------------------------
FilterSPtr EspINA::filterPtr(ModelItemSPtr& item)
{
  Q_ASSERT(EspINA::FILTER == item->type());
  FilterSPtr ptr = qSharedPointerDynamicCast<Filter>(item);
  Q_ASSERT(!ptr.isNull());

  return ptr;

}

//----------------------------------------------------------------------------
bool Filter::dumpSnapshot(Snapshot &snapshot)
{
  return false;
}

//----------------------------------------------------------------------------
void addChannelOutputData(ChannelOutputSPtr output, ChannelRepresentationSPtr rep)
{
  ChannelRepresentationSPtr oldRep = output->representation(rep->type());

  rep->setOutput(output.get());

  if (oldRep)
  {
    if (!oldRep->setInternalData(rep))
    {
      qWarning() << "Filter: Couldn't copy internal data";
      Q_ASSERT(false);
    }
  }
  else
    output->setRepresentation(rep->type(), rep);
}

//----------------------------------------------------------------------------
void ChannelFilter::createOutput(FilterOutputId id, ChannelRepresentationSPtr rep)
{
  if (!m_outputs.contains(id))
    m_outputs[id] = ChannelOutputSPtr(new ChannelOutput(this, id));

  ChannelOutputSPtr currentOutput = m_outputs[id];

  addChannelOutputData(currentOutput, rep);

  currentOutput->clearGraphicalRepresentations();

  createOutputRepresentations(currentOutput);
}

//----------------------------------------------------------------------------
void ChannelFilter::createOutput(FilterOutputId id, ChannelRepresentationSList repList)
{
  if (!m_outputs.contains(id))
    m_outputs[id] = ChannelOutputSPtr(new ChannelOutput(this, id));

  ChannelOutputSPtr currentOutput = m_outputs[id];

  foreach(ChannelRepresentationSPtr rep, repList)
  {
    addChannelOutputData(currentOutput, rep);
  }

  currentOutput->clearGraphicalRepresentations();//TODO: comprobar que se eliminan los actores de las antiguas representaciones en las vistas al ejecutar varios run

  createOutputRepresentations(currentOutput);
}

//----------------------------------------------------------------------------
void SegmentationFilter::setCacheDir(QDir dir)
{
  Filter::setCacheDir(dir);

  //WARNING: We need to create all output, even if they are invalid (NULL volume pointer)
  // Load cached outputs
  Q_ASSERT(m_outputs.isEmpty());
  // Compatibility: verion 3 seg files had only volume outputs shaved
  // in the base cache dir. Thus, in case we find some files there, we must
  // assign them to filter's volume outputs
  bool v3SegFile = false;

  QStringList namedFilters;
  namedFilters <<  QString("%1_*.mhd").arg(m_cacheId);
  foreach(QString cachedFile, m_cacheDir.entryList(namedFilters))
  {
    v3SegFile = true;

    QStringList ids = cachedFile.section(".",0,0).split("_");
    FilterOutputId oId = ids[1].toInt();

    if (!validOutput(oId))
    {
      createDummyOutput(oId, SegmentationVolume::TYPE);
    }

    SegmentationOutputSPtr editedOutput = m_outputs[oId];
    QString outputName = cacheOutputId(oId);

    if (editedOutput->editedRegions().isEmpty() && m_cacheDir.exists(outputName + ".trc"))
    {
      QFile file(m_cacheDir.absoluteFilePath(outputName + ".trc"));
      if (file.open(QIODevice::ReadOnly))
      {
        SegmentationVolumeSPtr volume = segmentationVolume(editedOutput);
        while (!file.atEnd())
        {
          QByteArray line = file.readLine();
          QStringList values = QString(line).split(" ");
          Nm bounds[6];
          for (int i = 0; i < 6; ++i)
            bounds[i] = values[i].toDouble();

          volume->addEditedRegion(EspinaRegion(bounds));
        }
      }
    }
  }

  if (!v3SegFile)
  {
    QDir outputsDir(m_cacheDir.filePath("Outputs"));

    QStringList namedFilters;
    namedFilters <<  QString("%1_*.trc").arg(m_cacheId);
    foreach(QString outputInfoFile, outputsDir.entryList(namedFilters))
    {
      QStringList ids = outputInfoFile.section(".",0,0).split("_");
      FilterOutputId oId = ids[1].toInt();

      QFile file(outputsDir.absoluteFilePath(outputInfoFile));
      if (file.open(QIODevice::ReadOnly))
      {
        bool header   = true;
        int  regionId = 0;
        while (!file.atEnd())
        {
          QString line = file.readLine();
          line = line.trimmed();
          if (header)
          {
            if (!line.isEmpty())
            {
              createDummyOutput(oId, line);
            } else
            {
              header = false;
            }
          } else // Restore edited regions
          {
            QStringList values = line.split(" ");
            Q_ASSERT(values.size() == 7);

            EspinaRegion region;
            for (int i = 0; i < 6; ++i)
              region[i] = values[i+1].toDouble();

            m_outputs[oId]->representation(values[0])->addEditedRegion(region, regionId++);
          }
        }
      }
    }
  }
}

//----------------------------------------------------------------------------
void addSegmentationRepresentation(SegmentationOutputSPtr output, SegmentationRepresentationSPtr rep)
{
  SegmentationRepresentationSPtr oldData = output->representation(rep->type());

  rep->setOutput(output.get());

  if (oldData)
  {
    if (!oldData->setInternalData(rep))
    {
      qWarning() << "Filter: Couldn't copy internal data";
      Q_ASSERT(false);
    }
  }
  else
    output->setRepresentation(rep->type(), rep);
}

//----------------------------------------------------------------------------
void SegmentationFilter::createOutput(FilterOutputId id, SegmentationRepresentationSPtr rep)
{
  if (!m_outputs.contains(id))
  {
    m_outputs[id] = SegmentationOutputSPtr(new SegmentationOutput(this, id));
    createDummyOutput(id, rep->type());
  }

  SegmentationOutputSPtr currentOutput = m_outputs[id];

  addSegmentationRepresentation(currentOutput, rep);

  currentOutput->clearGraphicalRepresentations();

  createOutputRepresentations(currentOutput);
}

//----------------------------------------------------------------------------
void SegmentationFilter::createOutput(FilterOutputId id, SegmentationRepresentationSList repList)
{
  if (!m_outputs.contains(id))
  {
    m_outputs[id] = SegmentationOutputSPtr(new SegmentationOutput(this, id));
    foreach(SegmentationRepresentationSPtr rep, repList)
    {
      createDummyOutput(id, rep->type());
    }
  }

  SegmentationOutputSPtr currentOutput = m_outputs[id];

  foreach(SegmentationRepresentationSPtr rep, repList)
  {
    addSegmentationRepresentation(currentOutput, rep);
  }

  currentOutput->clearGraphicalRepresentations();

  createOutputRepresentations(currentOutput);
}
