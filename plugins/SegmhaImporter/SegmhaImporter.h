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

#include <common/plugins/FilterFactory.h>
#include <common/plugins/ReaderFactory.h>
#include <QUndoCommand>

static const QString SFRF = "SegmhaReader::SegmhaImporterFilter";

class Channel;
class Sample;
class SegmhaImporterFilter;

/// Segmha Reader Plugin
class SegmhaImporter
      : public QObject
      , public FilterFactory
      , public ReaderFactory
{
  class UndoCommand : public QUndoCommand
  {
  public:
    explicit UndoCommand(SegmhaImporterFilter *filter);
    virtual void redo();
    virtual void undo();
  private:
    Sample               *m_sample;
    Channel              *m_channel;
    SegmhaImporterFilter *m_filter;
  };
public:
  SegmhaImporter(QObject* parent=0);

  void onStartup();
  void onShutdown(){}

  virtual Filter *createFilter(const QString filter, const QString args);
  virtual void readFile(const QString file);
};

#endif// SEGMHAIMPORTER_H