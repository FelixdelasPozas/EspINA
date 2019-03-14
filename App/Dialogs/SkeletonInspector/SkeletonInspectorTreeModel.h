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

#ifndef APP_DIALOGS_SKELETONINSPECTOR_SKELETONINSPECTORTREEMODEL_H_
#define APP_DIALOGS_SKELETONINSPECTOR_SKELETONINSPECTORTREEMODEL_H_

// ESPINA
#include <Core/Analysis/Data/SkeletonDataUtils.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Model/ViewItemAdapter.h>
#include <GUI/Types.h>

// Qt
#include <QAbstractItemModel>

// VTK
#include <vtkSmartPointer.h>

class vtkActor;

namespace ESPINA
{
  /** \struct StrokeInfo
   * \brief Holds stroke information needed for the skeleton inspector dialog.
   *
   */
  struct StrokeInfo
  {
     QString                          name;         /** stroke name.                       */
     Core::Path                       path;         /** stroke path.                       */
     double                           length;       /** stroke length.                     */
     bool                             used;         /** used in total length.              */
     QList<vtkSmartPointer<vtkActor>> actors;       /** stroke actors.                     */
     bool                             selected;     /** true if selected, false otherwise. */
     int                              hue;          /** hue color of the stroke.           */
     int                              randomHue;    /** random color.                      */
     int                              hierarchyHue; /** hierarchy color.                   */
     NmVector3                        labelPoint;   /** point of the label.                */

     /** \brief StrokeInfo struct empty constructor.
      *
      */
     StrokeInfo(): length{0}, used{false}, selected{false}, hue{0}, randomHue{0}, hierarchyHue{0} {};

     /** \brief Operator < for struct StrokeInfo.
      * \param[in] other Reference to a struct StrokeInfo to compare.
      *
      */
     bool operator<(const StrokeInfo &other) const
     {
       auto rdata = name;
       auto ldata = other.name;

       if(rdata.endsWith("(Truncated)", Qt::CaseInsensitive)) rdata = rdata.left(rdata.length()-12);
       if(ldata.endsWith("(Truncated)", Qt::CaseInsensitive)) ldata = ldata.left(ldata.length()-12);

       auto diff = rdata.length() - ldata.length();
       if(diff < 0) return true;
       if(diff > 0) return false;

       return (rdata < ldata);
     };

     /** \brief Operator == for struct StrokeInfo.
      * \param[in] other Reference to a struct StrokeInfo to compare.
      *
      */
     bool operator==(const StrokeInfo &other) const { return path == other.path; };
  };

  /** class SkeletonInspectorTreeModel
   * \brief Model for skeleton inspector tree view.
   *
   */
  class SkeletonInspectorTreeModel
  : public QAbstractItemModel
  {
      Q_OBJECT
    public:
      /** \brief SkeletonInspectorTreeModel class constructor.
       * \param[in] parent raw pointer to the QObject owner of this one.
       * \param[in] segmentation Segmentation adapter of the main segmentation of the tree.
       * \param[in] segmentations List of segmentation adapters of the direct and indirect connections.
       * \param[in] strokes List of stroke information structs.
       *
       */
      explicit SkeletonInspectorTreeModel(const SegmentationAdapterSPtr segmentation,
                                          ModelAdapterSPtr              model,
                                          QList<struct StrokeInfo>     &strokes,
                                          QObject                      *parent = nullptr);

      /** \brief SkeletonInspectorTreeModel class virtual destructor.
       *
       */
      virtual ~SkeletonInspectorTreeModel();

      /** \brief Displays the strokes with the normal color or with the random coloring.
       * \param[in] enabled True to use random coloring and false to use stroke color.
       *
       */
      void setRandomTreeColoring(const bool enabled);

      /** \brief Displays the strokes with the normal color or with the hierarchy coloring.
       * \param[in] enabled True to use hierarchy coloring and false to use stroke color.
       *
       */
      void setHierarchyTreeColoring(const bool enabled);

      QVariant data(const QModelIndex &index, int role) const override;

      Qt::ItemFlags flags(const QModelIndex &index) const override;

      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

      QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

      QModelIndex parent(const QModelIndex &index) const override;

      int rowCount(const QModelIndex &parent = QModelIndex()) const override;

      int columnCount(const QModelIndex &parent = QModelIndex()) const override;

      bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

      /** \brief Computes the connections part of the model given the adjacency distance.
       * \param[in] distance Distance in connections.
       *
       */
      void computeConnectionDistances(int distance);

      /** \struct SkeletonInspectorTreeModel::TreeNode
       * \brief Node of the internal data structure of the SkeletonInspectorTreeModel.
       *
       */
      struct TreeNode
      {
          enum class Type: char { STROKE = 0, SEGMENTATION, ROOT };

          Type              type;     /** type of node.                          */
          TreeNode         *parent;   /** parent of the node or nullptr if root. */
          void             *data;     /** data of the node or nullptr if root.   */
          QList<TreeNode *> children; /** list of children of the node.          */

          /** \brief TreeNode struct constructor.
           *
           */
          TreeNode(): type{Type::ROOT}, parent{nullptr}, data{nullptr} {}

          /** \brief TreeNode struct destructor.
           *
           */
          ~TreeNode()
          { for(auto node: children) delete node; }

          /** \brief Helper method that returns true if the node is a connection node.
           *
           */
          inline SegmentationAdapterPtr connection() const
          { Q_ASSERT(type != Type::STROKE); return static_cast<SegmentationAdapterPtr>(data); }

          /** \brief Helper method that returns true if the node is a stroke node.
           *
           */
          inline StrokeInfo *stroke() const
          { Q_ASSERT(type != Type::SEGMENTATION); return static_cast<StrokeInfo *>(data); }
      };

    public slots:
      /** \brief Updates the selected segmentations.
       * \param[in] segmentations Selected segmentations list.
       *
       */
      void onSelectionChanged(SegmentationAdapterList segmentations);

    signals:
      void invalidate(ViewItemAdapterList segmentations);
      void segmentationsShown(SegmentationAdapterList segmentations);

    private:
      /** \brief Helper method to compute the hierarchy of strokes.
       *
       */
      void computeStrokesTree();

      /** \brief Helper method to return the list of segmentations that connect to the given one.
       * \param[in] segmentation Segmentation smartpointer.
       *
       */
      SegmentationAdapterSList connections(const SegmentationAdapterPtr segmentation) const;

    private:
      /** \brief Changes the visibility of the data of the node and all its children, recursive.
       * \param[in] node Tree node.
       * \param[in] visible True to set visible false otherwise.
       */
      const SegmentationAdapterList setVisibility(TreeNode *node, bool visible);

      /** \brief Returns true if all nodes and subnodes are visible, and false otherwise.
       * \param[in] node Tree node.
       *
       */
      const bool getVisibility(TreeNode *node) const;

      /** \brief Helper method to obtain the tree node out of a QModelIndex.
       * \param[in] index Model index object.
       */
      inline TreeNode *dataNode(const QModelIndex &index) const
      { return static_cast<TreeNode *>(index.internalPointer()); }

    private:
      TreeNode                     *m_StrokesTree;          /** strokes tree structure.                                                 */
      TreeNode                     *m_ConnectTree;          /** connections tree.                                                       */
      const SegmentationAdapterSPtr m_segmentation;         /** Segmentation adapter of the main segmentation of the tree.              */
      ModelAdapterSPtr              m_model;                /** Application model adapter.                                              */
      QList<struct StrokeInfo>     &m_strokes;              /** List of stroke information structs.                                     */
      bool                          m_useRandomColoring;    /** true to color strokes with random color, false to use stroke color.     */
      bool                          m_useHierarchyColoring; /** true to color strokes with hierarchy colors, false to use stroke color. */
      unsigned int                  m_connectionLevel;      /** Connection level.                                                       */
      Core::SkeletonDefinition      m_definition;           /** skeleton definition struct.                                             */
  };

} // namespace ESPINA

#endif // APP_DIALOGS_SKELETONINSPECTOR_SKELETONINSPECTORTREEMODEL_H_
