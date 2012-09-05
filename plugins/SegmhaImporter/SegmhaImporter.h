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

#include <common/pluginInterfaces/FilterFactory.h>
#include <common/pluginInterfaces/ReaderFactory.h>
#include <QUndoCommand>


class Channel;
class Sample;
class Segmentation;
class SegmhaImporterFilter;

/// Segmha Reader Plugin
class SegmhaImporter
: public QObject
, public FilterFactory
, public ReaderFactory
{
  Q_OBJECT
  Q_INTERFACES(FilterFactory ReaderFactory)

  class UndoCommand
  : public QUndoCommand
  {
  public:
    explicit UndoCommand(Sample *sample,
			 Channel *channel,
			 SegmhaImporterFilter *filter);
    virtual void redo();
    virtual void undo();
  private:
    Sample               *m_sample;
    Channel              *m_channel;
    SegmhaImporterFilter *m_filter;
    QList<Segmentation *> m_segs;
  };

public:
  explicit SegmhaImporter();
  virtual ~SegmhaImporter(){}

  virtual Filter* createFilter(const QString filter,
			       Filter::NamedInputs inputs,
			       const ModelItem::Arguments args);
  virtual bool readFile(const QFileInfo file);
};

#endif// SEGMHAIMPORTER_H