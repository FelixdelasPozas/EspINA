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
#include <Core/Analysis/Data/SkeletonDataUtils.h>
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

      /** \brief Focus the view on the actor belonging to the given table index.
       * \param[in] row Table row position.
       *
       */
      void focusOnActor(int row);

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

      /** \brief Changes the coloring of strokes from stroke color to stroke hierachy colors.
       * \param[in] value True to display the strokes in hierarchy colors and false to use stroke color.
       *
       */
      void onColoringEnabled(bool value);

      /** \brief Changes the coloring of strokes from stroke color to random.
       * \param[in] value True to display the strokes in random colors and false to use stroke color.
       *
       */
      void onRandomColoringEnabled(bool value);

      /** \brief Updates the model and expands the connections subtree.
       * \param[in] distance Connections distance new value.
       */
      void onDistanceChanged(int distance);

      /** \brief Updates the view sources and connections when the segmentations distance changes in the model.
       * \param[in] segmentations List of segmentations in the distance minus the source one.
       *
       */
      void onSegmentationsShown(const SegmentationAdapterList segmentations);

      /** \brief Shows/hides the spine table when the button is clicked.
       * \param[in] checked true if checked and false otherwise.
       */
      void onSpinesButtonClicked(bool checked);

      /** \brief Selects the selected spine on the view.
       * \param[in] index Modelindex of the table.
       *
       */
      void onSpineSelected(const QModelIndex &index);

      /** \brief Shows the save data dialog and saves the table data to disk in the choosed format.
       *
       */
      void onSaveButtonPressed();

      /** \brief Updates the tree selection when a row in the table is selected.
       * \param[in] row Row of the activated cell.
       *
       */
      void onCellClicked(int row);

      /** \brief Saves the contents of the 3d scene to disk in VTK format. For debug purposes.
       *
       */
      void onSaveScene();

    private:
      /** \brief Creates the actors for the skeleton based on strokes.
       * \param[in] segmentation Skeleton segmentation object.
       *
       */
      void createSkeletonActors(const SegmentationAdapterPtr segmentation);

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

      /** \brief Initializes the spines table.
       *
       */
      void initSpinesTable();

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

      /** \brief Saves the spine table contents to the given file on disk in CSV format.
       * \param[in] filename Name of the file on disk.
       *
       */
      void saveToCSV(const QString &filename) const;

      /** \brief Saves the spine table contents to the given file on disk in Excel format.
       * \param[in] filename Name of the file on disk.
       *
       */
      void saveToXLS(const QString &filename) const;

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
           * \param[in] value True to enable random coloring, false otherwise.
           *
           */
          void setRandomColors(const bool value)
          { m_randomColoring = value; if(value && m_hierarchyColoring) m_hierarchyColoring = false; }

          /** \brief Returns true if random coloring is enabled, false if not.
           *
           */
          const bool randomColors() const
          { return m_randomColoring; }

          /** \brief Enables/Disables hierarchy coloring.
           * \param[in] value True to enable hierarchy coloring, false otherwise.
           *
           */
          void setHierarchyColors(const bool value)
          { m_hierarchyColoring = value; if(value && m_randomColoring) m_randomColoring = false; }

          /** \brief Returns true if hierarchy coloring is enabled, false otherwise.
           *
           */
          const bool hierarchyColors() const
          { return m_hierarchyColoring; }

        private:
          QList<struct StrokeInfo> &m_strokes;           /** stroke information structs list.                             */
          bool                      m_randomColoring;    /** true to use random colors and false to use stroke colors.    */
          bool                      m_hierarchyColoring; /** true to use hierarchy colors and false to use stroke colors. */
      };

      /** \class Item
       * \brief Reimplementation of a table widget item, needed for sorting the table.
       *
       */
      class Item
      : public QTableWidgetItem
      {
        public:
          /** \brief Item class constructor.
           *
           */
          explicit Item(const QString &info)
          : QTableWidgetItem{info}
          {};

          virtual bool operator<(const QTableWidgetItem &other) const override
          {
            if(column() == 0)
            {
              auto rdata = this->data(Qt::DisplayRole).toString();
              auto ldata = other.data(Qt::DisplayRole).toString();

              if(rdata.endsWith("(Truncated)", Qt::CaseInsensitive)) rdata = rdata.left(rdata.length()-12);
              if(ldata.endsWith("(Truncated)", Qt::CaseInsensitive)) ldata = ldata.left(ldata.length()-12);

              auto diff = rdata.length() - ldata.length();
              if(diff < 0) return true;
              if(diff > 0) return false;

              return (rdata < ldata);
            }

            return QTableWidgetItem::operator<(other);
          }
      };

      using TemporalPipelineSPtr = std::shared_ptr<SkeletonInspectorPipeline>;

      SegmentationAdapterSPtr   m_segmentation;        /** skeleton segmentation.                                 */
      View3D                    m_view;                /** 3D view.                                               */
      ManualPipelineSources     m_segmentationSources; /** list of channels as sources for pipelines.             */
      RepresentationList        m_representations;     /** list of view's representations factories and switches. */
      QList<struct StrokeInfo> *m_strokes;             /** list of stroke information.                            */
      TemporalPipelineSPtr      m_temporalPipeline;    /** segmentation temporal representation.                  */

      QMap<ViewItemAdapterPtr, Core::SkeletonDefinition> m_segDefinitions; /** skeleton definitions of view's segmentations. */
      QMap<ViewItemAdapterPtr, QList<struct StrokeInfo>> m_segStrokes;     /** strokes info of view's segmentations.         */
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
      void randomColoringEnabled(bool value);

    private slots:
      /** \brief Emits the correct signal depending on the button pressed.
       * \param[in] value True to enable and false to disable.
       *
       */
      void onButtonPressed(bool value);

    private:
      GUI::Widgets::ToolButton *m_coloring;       /** hierarchy coloring enable/disable button. */
      GUI::Widgets::ToolButton *m_coloringRandom; /** random coloring enable/disable button.    */
  };


} // namespace ESPINA

#endif // APP_DIALOGS_SKELETONINSPECTOR_SKELETONINSPECTOR_H_
