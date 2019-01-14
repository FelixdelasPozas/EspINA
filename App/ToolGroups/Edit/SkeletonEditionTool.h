/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef APP_TOOLGROUPS_EDIT_SKELETONEDITIONTOOL_H_
#define APP_TOOLGROUPS_EDIT_SKELETONEDITIONTOOL_H_

// ESPINA
#include <Core/Utils/Vector3.hxx>
#include <GUI/Model/ViewItemAdapter.h>
#include <GUI/Representations/Managers/TemporalManager.h>
#include <GUI/Representations/RepresentationPipeline.h>
#include <GUI/Representations/RepresentationState.h>
#include <GUI/Types.h>
#include <GUI/View/Widgets/Skeleton/SkeletonEventHandler.h>
#include <GUI/View/Widgets/Skeleton/SkeletonWidget2D.h>
#include <GUI/Widgets/ToolButton.h>
#include <Support/Context.h>
#include <Support/Widgets/EditTool.h>
#include <App/ToolGroups/Segment/Skeleton/SkeletonToolsEventHandler.h>
#include <ToolGroups/Segment/Skeleton/ConnectionPointsTemporalRepresentation2D.h>

// VTK
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

class QComboBox;

namespace ESPINA
{
  class DoubleSpinBoxAction;

  /** \class SkeletonEditionTool
   * \brief Tool for skeleton segmentation edition
   *
   */
  class SkeletonEditionTool
  : public Support::Widgets::EditTool
  {
      Q_OBJECT
    public:
      /** \brief SkeletonEditionTool class constructor.
       * \param[in] context application context.
       *
       */
      explicit SkeletonEditionTool(Support::Context &context);

      /** \brief SkeletonEditionTool class virtual destructor.
       *
       */
      virtual ~SkeletonEditionTool();

      virtual void abortOperation() override
      { if(isChecked()) deactivateEventHandler(); };

      virtual void saveSettings(std::shared_ptr<QSettings> settings) override;
      virtual void restoreSettings(std::shared_ptr<QSettings> settings) override;

      virtual void onExclusiveToolInUse(ProgressTool* tool) override;

    protected slots:
      /** \brief Enables/Disables the tool depending on the current segmentation selection.
       *
       */
      virtual void updateStatus() override;

    private slots:
      /** \brief Performs tool initialization/de-initialization.
       * \param[in] value true to initialize and false otherwise.
       *
       */
      void initTool(bool value);

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

      /** \brief Adds the cloned skeleton widget to the list of cloned and sets the parameters.
       * \param[in] clone cloned widget.
       *
       */
      void onSkeletonWidgetCloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr clone);

      /** \brief Adds the cloned skeleton widget to the list of cloned and sets the parameters.
       * \param[in] clone cloned widget.
       *
       */
      void onPointWidgetCloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr clone);

      /** \brief Updates the created segmentation.
       * \param[in] polydata skeleton data.
       *
       */
      void onSkeletonModified(vtkSmartPointer<vtkPolyData> polydata);

      /** \brief Updates the widget operation mode depending on the clicked buttons.
       * \param[in] value true if the sender button is checked and false otherwise.
       *
       */
      void onModeChanged(bool value);

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

      /** \brief Enables/disables the erase button depending on the value.
       * \param[in] value true if the modifier is pressed, and false when the modifier key is released.
       *
       */
      void onModifierPressed(bool value);

      /** \brief Updates the stroke type used in widgets.
       * \param[in] index new stroke index.
       *
       */
      void onStrokeTypeChanged(int index);

      /** \brief Shows the stroke configuration dialog.
       *
       */
      void onStrokeConfigurationPressed();

      /** \brief Performs a point check requested by the event handler.
       * \param[in] point Point 3D coordinates.
       *
       */
      void onPointCheckRequested(const NmVector3 &point);

      /** \brief Updates the UI when the stroke is changed by the widget.
       * \param[in] stroke Current stroke definition.
       *
       */
      void onStrokeChanged(const Core::SkeletonStroke stroke);

      /** \brief Changes the segmentation being edited or deactivates tool depending on the current selection.
       * \param[in] segmentations Currently selected segmentations list.
       *
       */
      void onSelectionChanged(SegmentationAdapterList segmentations);

      /** \brief Changes the color of strokes with the same color to facilitate visualization.
       * \param[in] value True to change the hue of strokes with the same color except the first one and false
       *  to draw all the strokes with the given hue.
       *
       */
      void onHueModificationsButtonClicked(bool value);

      /** \brief Deactivates the truncation button after a succesfull skeleton modification.
       *
       */
      void onTruncationSuccess();

    private:
      virtual bool acceptsNInputs(int n) const;

      virtual bool acceptsSelection(SegmentationAdapterList segmentations) override;

      virtual bool selectionIsNotBeingModified(SegmentationAdapterList segmentations) override;

      /** \brief Initializes and connects the representation factories.
       *
       */
      void initRepresentationFactories();

      /** \brief Initializes and connects the parameters widgets.
       *
       */
      void initParametersWidgets();

      /** \brief Helper method to configure the event handler for the tool.
       *
       */
      void initEventHandler();

      /** \brief Sets the operating mode of the widget depending on the state of the buttons.
       *
       */
      void updateWidgetsMode();

      /** \brief Updates the contents of the strokes combobox.
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
      using TemporalRepresentationsSPtr = GUI::Representations::Managers::TemporalPrototypesSPtr;
      using SkeletonWidgetSPtr          = GUI::View::Widgets::Skeleton::SkeletonWidget2DSPtr;

      bool                                                m_init;            /** true if the tool has been initialized.            */
      SkeletonToolsEventHandlerSPtr                       m_eventHandler;    /** tool's event handler.                             */
      GUI::Widgets::ToolButton                           *m_eraseButton;     /** Paint/erase button.                               */
      DoubleSpinBoxAction                                *m_minWidget;       /** min distance between points widget.               */
      DoubleSpinBoxAction                                *m_maxWidget;       /** max distance between points widget.               */
      GUI::Widgets::ToolButton                           *m_moveButton;      /** Move nodes button.                                */
      QComboBox                                          *m_strokeCombo;     /** stroke type combobox.                             */
      GUI::Widgets::ToolButton                           *m_strokeButton;    /** stroke configuration dialog button.               */
      GUI::Widgets::ToolButton                           *m_changeHueButton; /** Change coincident hue strokes during edition.     */
      GUI::Widgets::ToolButton                           *m_truncateButton;  /** Mark truncated branch button.                     */
      ViewItemAdapterPtr                                  m_item;            /** current element being created or channel in init. */
      TemporalRepresentationsSPtr                         m_factory;         /** representation prototypes.                        */
      TemporalRepresentationsSPtr                         m_pointsFactory;   /** representation prototypes.                        */
      QList<SkeletonWidgetSPtr>                           m_widgets;         /** list of widgets currently on views.               */
      QList<ConnectionPointsTemporalRepresentation2DSPtr> m_pointWidgets;    /** list of point representations currently on views. */
      bool                                                m_allowSwich;      /** true if the skeleton creation tool is enabled.    */
  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_EDIT_SKELETONEDITIONTOOL_H_
