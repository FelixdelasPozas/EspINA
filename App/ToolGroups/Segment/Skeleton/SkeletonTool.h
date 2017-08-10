/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_SKELETON_TOOL_H_
#define ESPINA_SKELETON_TOOL_H_

// ESPINA
#include <Core/Analysis/Data/SkeletonDataUtils.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/View/EventHandler.h>
#include <GUI/View/Widgets/EspinaWidget.h>
#include <GUI/EventHandlers/PointTracker.h>
#include <GUI/View/Widgets/Skeleton/SkeletonWidget2D.h>
#include <GUI/View/Widgets/Skeleton/SkeletonEventHandler.h>
#include <GUI/Widgets/CategorySelector.h>
#include <GUI/Widgets/ToolButton.h>
#include <Support/Widgets/ProgressTool.h>
#include <Support/Context.h>

// VTK
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

// Qt
#include <QAction>

class vtkPolyData;
class QUndoStack;

namespace ESPINA
{
  class DoubleSpinBoxAction;

  /** \class ManualFilterFactory
   * \brief Factory for SourceFilter filters.
   *
   */
  class SkeletonFilterFactory
  : public FilterFactory
  {
    public:
      static const Filter::Type SKELETON_FILTER;

      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type &filter, SchedulerSPtr scheduler) const override;

      virtual FilterTypeList providedFilters() const override;

    private:
      mutable DataFactorySPtr m_dataFactory; /** data factory for this factory. */
  };

  /** \class SkeletonTool
   * \brief Tool for skeleton segmentation creation
   *
   */
  class SkeletonTool
  : public Support::Widgets::ProgressTool
  {
      Q_OBJECT
    public:
      /** \brief SkeletonTool class constructor.
       * \param[in] context application context
       *
       */
      SkeletonTool(Support::Context &context);

      /** \brief SkeletonTool class virtual destructor.
       *
       */
      virtual ~SkeletonTool();

      virtual void abortOperation() override
      { deactivateEventHandler(); };

    private slots:
      /** \brief Performs tool initialization/de-initialization.
       * \param[in] value, true to initialize and false otherwise.
       *
       */
      void initTool(bool value);

      /** \brief Updates the widget with the new category properties.
       * \param[in] category CategoryAdapter smart pointer.
       *
       */
      void onCategoryChanged(CategoryAdapterSPtr category);

      /** \brief Updates the minimum point distance value in the widget when the value in the spinbox changes.
       * \param[in] value new minimum distance value.
       *
       */
      void onMinimumDistanceChanged(double value);

      /** \brief Updates the maximum point distance value in the widget when the value in the spinbox changes.
       * \param[in] value new maximum distance value.
       *
       */
      void onMaximumDistanceChanged(double value);

      /** \brief Updates the widget if the item being modified is removed from the model (i.e. by undo).
       * \param[in] segmentations List of segmentation adapter smart pointers removed from the model.
       *
       */
      void onSegmentationsRemoved(ViewItemAdapterSList segmentations);

      /** \brief Updates the minimum value of the tolerance widget.
       *
       */
      void onResolutionChanged();

      /** \brief Helper method to mark the tool un-initialized on model reset.
       *
       */
      void onModelReset();

      /** \brief Adds the cloned widget to the list of cloned and sets the parameters.
       * \param[in] clone cloned widget.
       *
       */
      void onWidgetCloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr clone);

      /** \brief Updates the created segmentation.
       * \param[in] polydata skeleton data.
       *
       */
      void onSkeletonModified(vtkSmartPointer<vtkPolyData> polydata);

      /** \brief Ends the current skeleton and starts a new one.
       *
       */
      void onNextButtonPressed();

      /** \brief Updates the stroke type of the widget when the stroke type changes.
       * \param[in] index current type combo box index.
       *
       */
      void onStrokeTypeChanged(int index);

      /** \brief Shows the stroke type definition dialog.
       *
       */
      void onStrokeConfigurationPressed();

      virtual void restoreSettings(std::shared_ptr<QSettings> settings) override;

      virtual void saveSettings(std::shared_ptr<QSettings> settings) override;

    private:
      /** \brief Initializes the filter factory.
       *
       */
      void initFilterFactory();

      /** \brief Initializes and connects the representation factory.
       *
       */
      void initRepresentationFactory();

      /** \brief Initializes and connects the parameters widgets.
       *
       */
      void initParametersWidgets();

      /** \brief Helper method to configure the event handler for the tool.
       *
       */
      void initEventHandler();

      /** \brief Updates the list of strokes in the strokes combobox.
       *
       */
      void updateStrokes();

    private:
      /** \class NullRepresentationPipeline
       * \brief Implements an empty representation.
       *
       */
      class NullRepresentationPipeline
      : public RepresentationPipeline
      {
        public:
          /** \brief NullRepresentationPipeline class constructor.
           *
           */
          explicit NullRepresentationPipeline()
          : RepresentationPipeline("SegmentationSkeleton2D")
          { /* representation type must be the same as the default one. */ }

          /** \brief NullRepresentationPipeline class virtual destructor.
           *
           */
          virtual ~NullRepresentationPipeline()
          {};

          virtual RepresentationPipeline::ActorList createActors(ConstViewItemAdapterPtr    item,
                                                                 const RepresentationState &state)
          { return RepresentationPipeline::ActorList(); }

          virtual  void updateColors(RepresentationPipeline::ActorList &actors,
                                     ConstViewItemAdapterPtr            item,
                                     const RepresentationState         &state)
          {}

          virtual bool pick(ConstViewItemAdapterPtr item, const NmVector3 &point) const
          { return false; }

          virtual RepresentationState representationState(ConstViewItemAdapterPtr    item,
                                                          const RepresentationState &settings)
          { return RepresentationState(); }
      };

    private:
      bool                                                      m_init;             /** true if the tool has been initialized.            */
      GUI::Widgets::CategorySelector                           *m_categorySelector; /** category selector widget.                         */
      DoubleSpinBoxAction                                      *m_minWidget;        /** min distance between points widget.               */
      DoubleSpinBoxAction                                      *m_maxWidget;        /** max distance between points widget.               */
      GUI::Widgets::ToolButton                                 *m_nextButton;       /** next segmentation button.                         */
      QComboBox                                                *m_strokeCombo;      /** stroke type combo box.                            */
      GUI::Widgets::ToolButton                                 *m_strokeButton;     /** stroke configuration dialog.                      */
      GUI::View::Widgets::Skeleton::SkeletonEventHandlerSPtr    m_eventHandler;     /** tool's event handler.                             */
      ViewItemAdapterPtr                                        m_item;             /** current element being created or channel in init. */
      GUI::Representations::Managers::TemporalPrototypesSPtr    m_factory;          /** representation prototypes.                        */
      QList<GUI::View::Widgets::Skeleton::SkeletonWidget2DSPtr> m_widgets;          /** list of widgets currently on views.               */
  };

  using SkeletonToolPtr  = SkeletonTool *;
  using SkeletonToolSPtr = std::shared_ptr<SkeletonTool>;

} // namespace EspINA

#endif // ESPINA_SKELETON_TOOL_H_
