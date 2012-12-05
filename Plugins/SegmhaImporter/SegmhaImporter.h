/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#ifndef SEGMHAIMPORTER_H
#define SEGMHAIMPORTER_H

#include <Core/Interfaces/IFactoryExtension.h>
#include <Core/Interfaces/IFilterCreator.h>
#include <Core/Interfaces/IFileReader.h>

#include <QUndoCommand>

class Channel;
class EspinaModel;
class Sample;
class Segmentation;
class SegmhaImporterFilter;
class ViewManager;

/// Segmha Reader Plugin
class SegmhaImporter
: public QObject
, public IFactoryExtension
, public IFileReader
, public IFilterCreator
{
  Q_OBJECT
  Q_INTERFACES(IFactoryExtension IFileReader IFilterCreator)

  class UndoCommand
  : public QUndoCommand
  {
  public:
    explicit UndoCommand(Sample *sample,
                         Channel *channel,
                         SegmhaImporterFilter *filter,
                         EspinaModel *model);
    virtual void redo();
    virtual void undo();
  private:
    EspinaModel          *m_model;
    Sample               *m_sample;
    Channel              *m_channel;
    SegmhaImporterFilter *m_filter;
    QList<Segmentation *> m_segs;
  };

public:
  explicit SegmhaImporter();
  virtual ~SegmhaImporter(){}

  virtual void initFactoryExtension(EspinaFactory* factory);

  virtual Filter *createFilter(const QString              &filter,
                               const Filter::NamedInputs  &inputs,
                               const ModelItem::Arguments &args);

  virtual void initFileReader(EspinaModel* model,
                              QUndoStack* undoStack,
                              ViewManager* vm);

  virtual bool readFile(const QFileInfo file);

private:
  EspinaModel *m_model;
  QUndoStack  *m_undoStack;
  ViewManager *m_viewManager;
};

#endif// SEGMHAIMPORTER_H