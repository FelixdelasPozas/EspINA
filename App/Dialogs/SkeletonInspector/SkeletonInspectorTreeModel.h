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
     QString                   name;              /** stroke name.                       */
     double                    length;            /** stroke length.                     */
     int                       numBranches;       /** number of branches.                */
     QString                   branchDistances;   /** distance between branches.         */
     QString                   branchAngles;      /** angles of branches.                */
     bool                      used;              /** used in total length.              */
     int                       connectionNum;     /** number of connections in stroke.   */
     QString                   connections;       /** connection points text.            */
     vtkSmartPointer<vtkActor> actor;             /** stroke actors.                     */
     bool                      selected;          /** true if selected, false otherwise. */
     int                       hue;               /** hue color of the stroke.           */

     /** \brief StrokeInfo struct empty constructor.
      *
      */
     StrokeInfo(): length{0}, numBranches{0}, used{false}, connectionNum{0}, selected{false}, hue{0} {};

     /** \brief Operator < for struct StrokeInfo.
      * \param[in] other Reference to a struct StrokeInfo to compare.
      *
      */
     bool operator<(const StrokeInfo &other) const { return name < other.name; };

     /** \brief Operator == for struct StrokeInfo.
      * \param[in] other Reference to a struct StrokeInfo to compare.
      *
      */
     bool operator==(const StrokeInfo &other) const { return name == other.name; };
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
      /** \bried SkeletonInspectorTreeModel class constructor.
       * \param[in] parent raw pointer to the QObject owner of this one.
       * \param[in] segmentation Segmentation adapter of the main segmentation of the tree.
       * \param[in] segmentations List of segmentation adapters of the direct and indirect connections.
       * \param[in] strokes List of stroke information structs.
       *
       */
      explicit SkeletonInspectorTreeModel(const SegmentationAdapterSPtr   segmentation,
                                          const SegmentationAdapterList  &segmentations,
                                          const QList<struct StrokeInfo> &strokes,
                                          QObject                        *parent = nullptr);

      /** \brief SkeletonInspectorTreeModel class virtual destructor.
       *
       */
      virtual ~SkeletonInspectorTreeModel()
      {};

      QVariant data(const QModelIndex &index, int role) const override;

      Qt::ItemFlags flags(const QModelIndex &index) const override;

      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

      QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

      QModelIndex parent(const QModelIndex &index) const override;

      int rowCount(const QModelIndex &parent = QModelIndex()) const override;

      int columnCount(const QModelIndex &parent = QModelIndex()) const override;

      bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    signals:
      void invalidate(ViewItemAdapterList segmentations);

    private:
      const SegmentationAdapterSPtr   m_segmentation; /** Segmentation adapter of the main segmentation of the tree.            */
      const SegmentationAdapterList  &m_connections;  /** List of segmentation adapters of the direct and indirect connections. */
      const QList<struct StrokeInfo> &m_strokes;      /** List of stroke information structs.                                   */
  };

} // namespace ESPINA

#endif // APP_DIALOGS_SKELETONINSPECTOR_SKELETONINSPECTORTREEMODEL_H_
