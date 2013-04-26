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
#include "VolumeOutputType.h"

// ITK
#include <itkRegionOfInterestImageFilter.h>
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
  //WARNING: We need to create all output, even if they are invalid (NULL volume pointer)
  m_cacheDir = dir;

  // Load cached outputs
  if (m_outputs.isEmpty())
  {
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
        createDummyOutput(oId, EspinaVolume::TYPE);
      }

      OutputSPtr editedOutput = m_outputs[oId];
      QString outputName = QString("%1_%2").arg(m_cacheId).arg(oId);

      if (editedOutput->editedRegions().isEmpty() && m_cacheDir.exists(outputName + ".trc"))
      {
        QFile file(m_cacheDir.absoluteFilePath(outputName + ".trc"));
        if (file.open(QIODevice::ReadOnly))
        {
          VolumeOutputTypeSPtr volume = outputVolume(editedOutput);
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
  }
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
void Filter::draw(FilterOutputId oId,
                  vtkImplicitFunction* brush,
                  const Nm bounds[6],
                  itkVolumeType::PixelType value,
                  bool emitSignal)
{
  EspinaRegion region(bounds);

  EspinaVolumeSPtr volume = outputEspinaVolume(output(oId));

  if (volume)
  {
    volume->draw(brush, bounds, value, emitSignal);

    emit modified(this);
  }
}

//----------------------------------------------------------------------------
void Filter::draw(FilterOutputId oId,
                  itkVolumeType::IndexType index,
                  itkVolumeType::PixelType value,
                  bool emitSignal)
{
  EspinaVolumeSPtr volume = outputEspinaVolume(output(oId));

  if (volume)
  {
    volume->draw(index, value, emitSignal);

    emit modified(this);
  }
}

//----------------------------------------------------------------------------
void Filter::draw(FilterOutputId oId,
                  Nm x, Nm y, Nm z,
                  itkVolumeType::PixelType value,
                  bool emitSignal)
{
  EspinaVolumeSPtr volume = outputEspinaVolume(output(oId));

  if (volume)
  {
    volume->draw(x, y, z, value, emitSignal);

    emit modified(this);
  }
}

//----------------------------------------------------------------------------
void Filter::draw(FilterOutputId oId,
                  vtkPolyData *contour,
                  Nm slice, PlaneType plane,
                  itkVolumeType::PixelType value,
                  bool emitSignal)
{
  if (contour->GetPoints()->GetNumberOfPoints() == 0)
    return;

  EspinaVolumeSPtr volume = outputEspinaVolume(output(oId));

  if (volume)
  {
    volume->draw(contour, slice, plane, value, emitSignal);

    emit modified(this);
  }
}

//----------------------------------------------------------------------------
void Filter::draw(FilterOutputId oId,
                  itkVolumeType::Pointer volume,
                  bool emitSignal)
{
  EspinaVolumeSPtr filterVolume = outputEspinaVolume(output(oId));

  if (filterVolume)
  {
    filterVolume->draw(volume, emitSignal);

    emit modified(this);
  }
}

//----------------------------------------------------------------------------
void Filter::fill(FilterOutputId oId,
                  itkVolumeType::PixelType value,
                  bool emitSignal)
{
  EspinaVolumeSPtr volume = outputEspinaVolume(output(oId));

  if (volume)
  {
    volume->fill(value, emitSignal);

    emit modified(this);
  }
}

//----------------------------------------------------------------------------
void Filter::fill(FilterOutputId oId,
                  const EspinaRegion &region,
                  itkVolumeType::PixelType value,
                  bool emitSignal)
{
  EspinaVolumeSPtr volume = outputEspinaVolume(output(oId));

  if (volume)
  {
    volume->fill(region, value, emitSignal);

    emit modified(this);
  }
}

//----------------------------------------------------------------------------
void Filter::restoreOutput(FilterOutputId oId, itkVolumeType::Pointer volume)
{
  EspinaVolumeSPtr filterVolume = outputEspinaVolume(output(oId));

  if (filterVolume)
  {
    filterVolume->setVolume(volume);
    Q_ASSERT(false);
//     filterVolume->markAsModified();
    emit modified(this);
  }
}

//----------------------------------------------------------------------------
bool Filter::validOutput(FilterOutputId oId)
{
  return m_outputs.contains(oId);
}

//----------------------------------------------------------------------------
const OutputSPtr Filter::output(FilterOutputId oId) const
{
  return m_outputs.value(oId, OutputSPtr());
}

//----------------------------------------------------------------------------
OutputSPtr Filter::output(FilterOutputId oId)
{
  return m_outputs.value(oId, OutputSPtr());
}

//----------------------------------------------------------------------------
bool Filter::needUpdate() const
{
  bool update = true;

  if (!m_outputs.isEmpty())
  {
    update = false;
    foreach(OutputSPtr filterOutput, m_outputs)
    {
      update = update || !filterOutput->isValid();
    }
  }

  return update;
}

//----------------------------------------------------------------------------
bool Filter::needUpdate(FilterOutputId oId) const
{
  bool update = true;

  if (!m_outputs.isEmpty())
  {
    update = !output(oId)->isValid();
  }

  return update;
}

//----------------------------------------------------------------------------
void Filter::update()
{
  if (m_outputs.isEmpty())
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
    foreach(FilterOutputId oId, m_outputs.keys())
    {
      update(oId);
    }
  }
}

//----------------------------------------------------------------------------
 void Filter::update(FilterOutputId oId)
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
         QString outputPrefix = QString("%1_%2").arg(m_cacheId).arg(oId);
         foreach(FilterOutput::NamedRegion region, m_outputs[oId]->editedRegions())
         {
           m_outputs[oId]->data(region.first)->restoreEditedRegion(this, region.second, outputPrefix);
         }
       }

       m_executed = true;
     }
   }
 }

// //----------------------------------------------------------------------------
// void Filter::createOutput(FilterFilterOutputId id, EspinaVolume::Pointer volume)
// {
//   if (!m_outputs.contains(id))
//     m_outputs[id] = Output(this, id, volume);
//   else if (volume)
//     m_outputs[id].volume->setVolume(volume->toITK());
// }

//----------------------------------------------------------------------------
bool Filter::fetchSnapshot(FilterOutputId oId)
{
  if (m_outputs.isEmpty()) // contains??
    return false;

  Q_ASSERT(m_outputs.contains(oId));

  QString filterPrefix = QString("%1_%2").arg(m_cacheId).arg(oId);

  bool fetched = m_outputs[oId]->fetchSnapshot(filterPrefix);

  if (fetched)
    emit modified(this);

  return fetched;
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

void addOutputData(OutputSPtr output, FilterOutput::OutputTypeSPtr data)
{
  FilterOutput::OutputTypeSPtr out = output->data(data->type());

  data->setOutput(output.get());

  if (out)
  {
    if (!out->setInternalData(data))
    {
      qWarning() << "Filter: Couldn't copy internal data";
      Q_ASSERT(false);
    }
  }
  else
    output->setData(data->type(), data);
}

//----------------------------------------------------------------------------
void Filter::createOutput(FilterOutputId id, FilterOutput::OutputTypeSPtr data)
{
  if (!m_outputs.contains(id))
    m_outputs[id] = OutputSPtr(new FilterOutput(this, id));

  OutputSPtr currentOutput = m_outputs[id];

  addOutputData(currentOutput, data);

  createOutputRepresentations(currentOutput);
}

//----------------------------------------------------------------------------
void Filter::createOutput(FilterOutputId id, FilterOutput::OutputTypeList dataList)
{
  if (!m_outputs.contains(id))
    m_outputs[id] = OutputSPtr(new FilterOutput(this, id));

  OutputSPtr currentOutput = m_outputs[id];

  foreach(FilterOutput::OutputTypeSPtr data, dataList)
  {
    addOutputData(currentOutput, data);
  }

  createOutputRepresentations(currentOutput);
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

typedef itk::RegionOfInterestImageFilter<itkVolumeType, itkVolumeType> ROIFilter;


//----------------------------------------------------------------------------
bool Filter::dumpSnapshot(Snapshot &snapshot)
{
  QDir temporalDir = QDir::tempPath();
  bool result = false;

  foreach(OutputSPtr output, this->outputs())
  {
    QString filterPrefix = temporalDir.absoluteFilePath(QString("%1_%2").arg(id()).arg(output->id()));

    if (output->isCached())
    {
      update(output->id());

      result |= output->dumpSnapshot(filterPrefix, snapshot);
    }

    // Backwards compatibility with seg files version 1.0
    if (m_model->isTraceable() && m_args.contains(EDIT))
    {
      update(output->id());

      QStringList outputIds = m_args[EDIT].split(",");
      if (outputIds.contains(QString::number(output->id())))
      {
        EspinaVolumeSPtr editedVolume = outputEspinaVolume(output);
        EspinaRegion region = editedVolume->espinaRegion();
        editedVolume->addEditedRegion(region);
      }
    }

    if (m_model->isTraceable() && output->isEdited())
    {
      result = true;
      output->dumpSnapshot(filterPrefix, snapshot);
    }

    output->setCached(false);
  }

  return result;
}
