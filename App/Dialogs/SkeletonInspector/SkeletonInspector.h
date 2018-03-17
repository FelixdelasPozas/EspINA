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

#ifndef APP_DIALOGS_SKELETONINSPECTOR_SKELETONINSPECTOR_H_
#define APP_DIALOGS_SKELETONINSPECTOR_SKELETONINSPECTOR_H_

// ESPINA
#include "ui_SkeletonInspector.h"
#include "SkeletonInspectorTreeModel.h"
#include <App/ToolGroups/Visualize/Representations/Switches/SegmentationSkeletonSwitch.h>
#include <GUI/Representations/ManualPipelineSources.h>
#include <GUI/Representations/Pipelines/SegmentationSkeleton3DPipeline.h>
#include <GUI/View/View3D.h>
#include <GUI/Representations/RepresentationManager.h>
#include <GUI/Representations/Settings/SegmentationSkeletonPoolSettings.h>
#include <Support/Context.h>
#include <Support/Representations/RepresentationFactory.h>
#include <Support/Types.h>

// Qt
#include <QDialog>
#include <QToolBar>

namespace ESPINA
{
  /** \class SkeletonInspector
   * \brief Dialog to show information about a given skeleton.
   *
   */
  class SkeletonInspector
  : public QDialog
  , private Support::WithContext
  , private Ui::SkeletonInspector
  {
      Q_OBJECT
    public:
      /** \brief SkeletonInspector class constructor.
       * \param[in] context Application context.
       *
       */
      explicit SkeletonInspector(Support::Context& context);

      /** \brief SkeletonInspector class virtual destructor.
       *
       */
      virtual ~SkeletonInspector()
      {};

    signals:
      void connectionAdded(Connection connection);
      void connectionRemoved(Connection connection);
      void aboutToBeReset();

    protected:
      void showEvent(QShowEvent *event) override;
      void closeEvent(QCloseEvent *event) override;

    private slots:
      /** \brief Focus the view on the actor belonging to the given index.
       * \param[in] index index of the actor in the tree view.
       *
       */
      void focusOnActor(QModelIndex index);

      /** \brief Updates the actor selection on the view depeding on the currently selected item.
       * \param[in] current index of the currently selected item.
       * \param[in] previous index of the previously selected item.
       *
       */
      void onCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

      /** \brief Invalidates the representations of the given segmentations.
       * \param[in] segmentations list of segmentation adapter pointers.
       *
       */
      void onRepresentationsInvalidated(ViewItemAdapterList segmentations);

      /** \brief Changes the coloring of strokes from stroke color to random.
       * \param[in] value True to display the strokes in random colors and false to use stroke color.
       *
       */
      void onColoringEnabled(bool value);

      /** \brief Updates the model and expands the connections subtree.
       * \param[in] distance Connections distance new value.
       */
      void onDistanceChanged(int distance);

      /** \brief Updates the view sources and connections when the segmentations distance changes in the model.
       * \param[in] segmentations List of segmentations in the distance minus the source one.
       *
       */
      void onSegmentationsShown(const SegmentationAdapterList segmentations);

    private:
      /** \brief Creates the actors for the skeleton based on strokes.
       * \param[in] segmentation Skeleton segmentation object.
       *
       */
      void createSkeletonActors(const SegmentationAdapterSPtr segmentation);

      /** \brief Updates the window title with the name of the inspected skeleton segmentation.
       * \param[in] segmentationName Skeleton segmentation name.
       *
       */
      void updateWindowTitle(const QString &segmentationName);

      /** \brief Gets the geometry of the window from the registry and updates the window.
       *
       */
      void restoreGeometry();

      /** \brief Initializes the 3D view with the given representation factories.
       * \param[in] representations Representations factories and buttons.
       *
       */
      void initView3D(RepresentationFactorySList representations);

      /** \brief Helper method to initialize the tree view.
       *
       */
      void initTreeView();

      /** \brief Adds the currently selected segmentation and it's connected ones at a distance 1 to the view and the sources.
       *
       */
      void addInitialSegmentations();

      /** \brief Adds the given segmentations to the view's sources.
       * \param[in] segmentations Segmentations list.
       *
       */
      void addSegmentations(const SegmentationAdapterList &segmentations);

      /** \brief Helper method that emits all the connections for the ConnectionManager.
       *
       */
      void emitSegmentationConnectionSignals();

    private:
      class SkeletonInspectorRepresentationSwitch;

      /** \class SkeletonInspectorPipeline
       * \brief Custom skeleton 3D representation pipeline for the skeleton inspector.
       *
       */
      class SkeletonInspectorPipeline
      : public GUI::Representations::SegmentationSkeleton3DPipeline
      {
        public:
          /** \brief SkeletonInspectorPipeline class constructor.
           * \param[in] strokes list of stroke information and actors.
           *
           */
          explicit SkeletonInspectorPipeline(QList<struct StrokeInfo> &strokes);

          /** \brief SkeletonInspectorPipeline class virtual destructor.
           *
           */
          virtual ~SkeletonInspectorPipeline()
          {};

          virtual  void updateColors(RepresentationPipeline::ActorList &actors,
                                     ConstViewItemAdapterPtr            item,
                                     const RepresentationState         &state) override final;

          virtual bool pick(ConstViewItemAdapterPtr item, const NmVector3 &point) const override final;


          virtual RepresentationPipeline::ActorList createActors(ConstViewItemAdapterPtr    item,
                                                                 const RepresentationState &state) override final;

          /** \brief Enables/Disables random coloring.
           * \param[in] value True to enable random coloring and false to use stroke colors.
           *
           */
          void setRandomColors(const bool value)
          { m_randomColoring = value; }

          /** \brief Returns true if random coloring is enabled and false if the stroke colors are being used.
           *
           */
          const bool randomColors() const
          { return m_randomColoring; }

        private:
          QList<struct StrokeInfo> &m_strokes;        /** stroke information structs list.                          */
          bool                      m_randomColoring; /** true to use random colors and false to use stroke colors. */
      };

      using TemporalPipelineSPtr = std::shared_ptr<SkeletonInspectorPipeline>;

      SegmentationAdapterSPtr  m_segmentation;        /** skeleton segmentation.                                 */
      View3D                   m_view;                /** 3D view.                                               */
      ManualPipelineSources    m_segmentationSources; /** list of channels as sources for pipelines.             */
      RepresentationList       m_representations;     /** list of view's representations factories and switches. */
      QList<struct StrokeInfo> m_strokes;             /** list of stroke information.                            */
      TemporalPipelineSPtr     m_temporalPipeline;    /** segmentation temporal representation.                  */
  };

  /** \class SkeletonInspectorRepresentationSwitch
   * \brief Switch that inherits from the skeleton switch just to add a new coloring switch that is exclusive to the inspector.
   *
   */
  class SkeletonInspector::SkeletonInspectorRepresentationSwitch
  : public SegmentationSkeletonSwitch
  {
      Q_OBJECT
    public:
      /** \brief SkeletonInspectorRepresentationSwitch class constructor.
       * \param[in] manager Manager associated with this switch.
       * \param[in] settings Skeleton representation settings.
       * \param[in] context Application context.
       *
       */
      explicit SkeletonInspectorRepresentationSwitch(GUI::Representations::RepresentationManagerSPtr manager,
                                                     GUI::Representations::Settings::SkeletonPoolSettingsSPtr settings,
                                                     Support::Context& context);

      /** \brief SkeletonInspectorRepresentationSwitch class virtual destructor.
       *
       */
      virtual ~SkeletonInspectorRepresentationSwitch()
      {}

    signals:
      void coloringEnabled(bool value);

    private:
      GUI::Widgets::ToolButton *m_coloring; /** random coloring enable/disable button. */
  };


} // namespace ESPINA

#endif // APP_DIALOGS_SKELETONINSPECTOR_SKELETONINSPECTOR_H_
