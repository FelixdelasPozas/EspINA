/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SEGMHAIMPORTER_H
#define SEGMHAIMPORTER_H

#include <Core/Interfaces/IFactoryExtension.h>
#include <Core/Interfaces/IFilterCreator.h>
#include <Core/Interfaces/IFileReader.h>

#include "SegmhaImporterFilter.h"
#include <QUndoCommand>

namespace EspINA
{
  class ViewManager;

  /// Segmha Reader Plugin
  class SegmhaImporter
  : public QObject
  , public IFactoryExtension
  , public IFilterCreator
  , public IFileReader
  {
    Q_OBJECT
    Q_INTERFACES
    (
      EspINA::IFactoryExtension
      EspINA::IFilterCreator
      EspINA::IFileReader
    )

    class UndoCommand
    : public QUndoCommand
    {
    public:
      static const Filter::FilterType FILTER_TYPE;
    public:
      explicit UndoCommand(SampleSPtr          sample,
                           ChannelSPtr         channel,
                           SegmhaImporterFilterSPtr filter,
                           EspinaModel         *model,
                           QUndoCommand        *parent = NULL);
      virtual void redo();
      virtual void undo();

    private:
      EspinaModel         *m_model;
      SampleSPtr          m_sample;
      ChannelSPtr         m_channel;
      SegmhaImporterFilterSPtr m_filter;
      SegmentationSList   m_segs;
    };

  public:
    explicit SegmhaImporter();
    virtual ~SegmhaImporter();

    virtual void initFactoryExtension(EspinaFactory *factory);

    virtual FilterSPtr createFilter(const QString              &filter,
                                    const Filter::NamedInputs  &inputs,
                                    const ModelItem::Arguments &args);

    virtual void initFileReader(EspinaModel *model,
                                QUndoStack  *undoStack,
                                ViewManager *viewManager);

    virtual bool readFile(const QFileInfo file, EspinaIO::ErrorHandler *handler = NULL);

  private:
    EspinaModel *m_model;
    QUndoStack  *m_undoStack;
    ViewManager *m_viewManager;
  };

} // namespace EspINA

#endif// SEGMHAIMPORTER_H
