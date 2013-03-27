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

#ifndef TUBULARSOURCE_H
#define TUBULARSOURCE_H

// EspINA
#include <Core/Model/Filter.h>
#include <Core/EspinaTypes.h>
#include "itkImplicitImageSource.h"

// Qt
#include <QUndoStack>

class vtkImplicitFunction;
class QVector4D;

namespace EspINA
{
  class ViewManager;
  class TubularWidget;
  class TubularTool;
  class TubularFilterInspector;

  class TubularSegmentationFilter
  : public SegmentationFilter
  {
      typedef itk::ImplicitImageSource<itkVolumeType> ImplicitSource;

    public:
      typedef QSharedPointer<TubularSegmentationFilter> Pointer;

    public:
      static const QString FILTER_TYPE;

      static const ModelItem::ArgumentId SPACING;
      static const ModelItem::ArgumentId NODES;

      typedef QList<QVector4D> NodeList;

      class Parameters
      {
        public:
          explicit Parameters(Arguments &args);

          void setSpacing(double value[3]);
          itkVolumeType::SpacingType spacing() const { return m_spacing; }

          void setNodeList(TubularSegmentationFilter::NodeList nodes);
          TubularSegmentationFilter::NodeList nodeList() { return m_nodes; }

        private:
          Arguments                          &m_args;
          itkVolumeType::SpacingType          m_spacing;
          TubularSegmentationFilter::NodeList m_nodes;
      };

    public:
      explicit TubularSegmentationFilter(NamedInputs inputs = NamedInputs(),
                                         Arguments args = Arguments(),
                                         Filter::FilterType type = Filter::FilterType());
      virtual ~TubularSegmentationFilter();

      void setNodes(NodeList nodes);
      NodeList nodes() { return m_param.nodeList(); }

      /// Implements Model Item Interface
      virtual QVariant data(int role = Qt::DisplayRole) const;

      /// Implements Filter Interface
      virtual bool needUpdate(OutputId oId) const;
      virtual bool fetchSnapshot(OutputId oId);

      // get/set round segmentation extremes
      virtual void setRoundedExtremes(bool value);
      virtual bool getRoundedExtremes();

      // get/set filter lazy execution
      virtual void setLazyExecution(bool value);
      virtual bool getLazyExecution();
      virtual void executeFilter();

      virtual void setTool(TubularTool *);

    protected:
      virtual void run() { updateVolume(); }
      void updateVolume();

    private:
      Parameters m_param;
      int Extent[6];
      int DrawExtent[6];
      bool RoundedExtremes;
      bool LazyExecution;

      ImplicitSource::Pointer m_filter;
      itkVolumeType::SpacingType m_spacing;
      EspinaVolumeReader::Pointer m_cachedFilter;
      ImplicitSource::FunctionList m_implicitFunctions;
      TubularFilterInspector *m_filterInspector;
      TubularTool *m_tool;

      friend class TubularSourceSetupWidget;
  };
}

#endif // TUBULARSOURCE_H
