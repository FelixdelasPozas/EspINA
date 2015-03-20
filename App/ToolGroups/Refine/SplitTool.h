/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_SPLIT_TOOL_H_
#define ESPINA_SPLIT_TOOL_H_

// ESPINA
#include <GUI/Model/ModelAdapter.h>
#include <GUI/ModelFactory.h>
#include <GUI/View/Widgets/PlanarSplit/PlanarSplitWidget.h>
#include <GUI/Widgets/ActionSelector.h>
#include <Support/Widgets/Tool.h>
#include <Support/ViewManager.h>

// Qt
#include <QUndoStack>

class QAction;

namespace ESPINA
{
  class SplitToolEventHandler;
  using SplitToolEventHandlerSPtr = std::shared_ptr<SplitToolEventHandler>;

  class SplitTool
  : public Tool
  {
    Q_OBJECT

    class SplitFilterFactory
    : public FilterFactory
    {
      virtual FilterTypeList providedFilters() const;

      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const throw (Unknown_Filter_Exception);

      private:
        mutable DataFactorySPtr m_dataFactory;
    };

    public:
      /** \brief SplitTool class constructor.
       * \param[in] model model adapter smart pointer.
       * \param[in] factory factory smart pointer.
       * \param[in] viewManager view manager smart pointer.
       * \param[in] undoStack QUndoStack object raw pointer.
       *
       */
      SplitTool(ModelAdapterSPtr model,
                ModelFactorySPtr factory,
                ViewManagerSPtr  viewManager,
                QUndoStack      *undoStack);

      /** \brief SplitTool class virtual destructor.
       *
       */
      virtual ~SplitTool();

      virtual QList<QAction *> actions() const override;

      virtual void abortOperation() override;

    signals:
      void splittingStopped();

    public slots:
      /** \brief Initializes/De-initializes the tool.
       * \param[in] enable, boolean value indicating the activation of the tool.
       */
      void initTool(bool enable);

      /** \brief Splits the segmentation using the current state of the tool.
       *
       */
      void applyCurrentState();

      /** \brief Creates the segmentations and adds them to the model.
       *
       */
      void createSegmentations();

      /** \brief Stops current operation.
       *
       */
      void stopSplitting()
      { initTool(false); }

  private:
    virtual void onToolEnabled(bool enabled);

    private:
      struct Data
      {
        FilterSPtr              adapter;
        SegmentationAdapterSPtr segmentation;

        Data(FilterSPtr adapterP, SegmentationAdapterSPtr segmentationP)
        : adapter{adapterP}, segmentation{segmentationP}
        {};

        Data(): adapter{nullptr}, segmentation{nullptr}
        {};
      };

      QAction *m_planarSplitAction;
      QAction *m_applyButton;

      ModelAdapterSPtr m_model;
      ModelFactorySPtr m_factory;
      ViewManagerSPtr  m_viewManager;
      QUndoStack      *m_undoStack;

      EspinaWidgetSPtr m_widget;
      SplitToolEventHandlerSPtr m_handler;
      QMap<FilterPtr, struct Data> m_executingTasks;
  };

  using SplitToolPtr  = SplitTool *;
  using SplitToolSPtr = std::shared_ptr<SplitTool>;

  class SplitToolEventHandler
  : public EventHandler
  {
    public:
      /** \brief SplitToolEventHandler class constructor.
       *
       */
      explicit SplitToolEventHandler();

      /** \brief SplitToolEventHandler class destructor.
       *
       */
      ~SplitToolEventHandler()
      {}

      virtual bool filterEvent(QEvent *e, RenderView *view = nullptr) override;
  };

} // namespace ESPINA

#endif // ESPINA_SPLIT_TOOL_H_
