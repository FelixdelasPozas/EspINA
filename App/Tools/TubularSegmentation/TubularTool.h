/*
 * SpineTool.h
 *
 *  Created on: Oct 24, 2012
 *      Author: Felix de las Pozas Álvarez
 */

#ifndef TUBULARTOOL_H_
#define TUBULARTOOL_H_

// EspINA
#include <GUI/Tools/ITool.h>
#include <GUI/Pickers/PixelSelector.h>
#include <Core/Model/EspinaModel.h>

// Spines plugin
#include <GUI/vtkWidgets/vtkTubularWidget.h>
#include <Core/Filters/TubularSegmentationFilter.h>

class QUndoStack;

namespace EspINA
{
  class ISelector;
  class Channel;
  class ModelAdapter;
  class Segmentation;
  class ViewManager;
  class TubularWidget;
  class PixelSelector;

  class TubularTool
  : public ITool
  {
    Q_OBJECT
    public:
      class UpdateSegmentationNodes
      : public QUndoCommand
      {
        public:
          explicit UpdateSegmentationNodes(TubularSegmentationFilter::Pointer filter,
                                           TubularSegmentationFilter::NodeList nodes);
          virtual void redo();
          virtual void undo();
        private:
          TubularSegmentationFilter::Pointer  m_filter;
          TubularSegmentationFilter::NodeList m_nodes;
          TubularSegmentationFilter::NodeList m_prevNodes;
      };

    public:
      explicit TubularTool(ViewManager *, QUndoStack *, ModelAdapter *);
      virtual ~TubularTool();

      virtual QCursor cursor() const;
      virtual bool filterEvent(QEvent *e, EspinaRenderView *view = NULL);
      virtual void setInUse(bool enable);
      virtual bool isInUse();
      virtual void setEnabled(bool enable);
      virtual bool enabled() const;

      virtual void Reset();
      virtual void showSpineInformation();
      virtual void setFilter(TubularSegmentationFilter::Pointer);
      virtual void setNodes(TubularSegmentationFilter::NodeList);

      // get/set for lazy execution of the filter
      virtual bool getLazyExecution();
      virtual void setLazyExecution(bool);

      virtual void setRoundedExtremes(bool);
      virtual bool getRoundedExtremes();

    public slots:
      void updateNodes(TubularSegmentationFilter::NodeList);
      void pixelSelected(ISelector::PickList);

    signals:
      void segmentationStopped();

    private:
      ChannelSPtr                         m_channel;
      SegmentationSPtr                    m_seg;
      ModelAdapter                        *m_model;
      bool                                m_enabled;
      bool                                m_inUse;
      bool                                m_roundExtremes;
      boost::shared_ptr<PixelSelector>         m_toolPicker;
      TubularWidget                      *m_widget;
      ViewManager                        *m_viewManager;
      QUndoStack                         *m_undoStack;
      TubularSegmentationFilter::Pointer  m_source;
  };

  typedef TubularTool*                TubularToolPtr;
  typedef boost::shared_ptr<TubularTool> TubularToolSPtr;
}
#endif /* TUBULARTOOL_H_ */
