/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef QCOMBOTREEVIEW_H
#define QCOMBOTREEVIEW_H

#include "GUI/EspinaGUI_Export.h"

// Qt
#include <QComboBox>
#include <QTreeView>

class QTreeView;

class EspinaGUI_EXPORT QComboTreeView
: public QComboBox
{
  Q_OBJECT
public:
  /* \brief QComboTreeView class constructor.
   * \param[in] parent, raw pointer of the QWidget parent of this one.
   *
   */
  explicit QComboTreeView(QWidget* parent = nullptr);

  /* \brief Shadows QComboBox::setModel().
   *
   */
  void setModel(QAbstractItemModel *model);

  /* \brief Shadows QComboBox::setModelIndex().
   *
   */
  void setRootModelIndex( const QModelIndex &index);

  /* \brief Sets the current model index as the given one.
   * \param[in] index, model index.
   *
   */
  void setCurrentModelIndex(const QModelIndex &index);

  /* \brief Returns the current model index.
   *
   */
  QModelIndex currentModelIndex() const
  {return m_currentModelIndex;}

  /* \brief Overrides QComboxBox::mousePressEvent().
   *
   */
  virtual void mousePressEvent(QMouseEvent* e) override;

protected:
  /* \brief Overrides QComboBox::showPopup().
   *
   */
  virtual void showPopup() override;

private slots:
	/* \brief Sets the current model index to the given one.
	 * \param[in] index, model index.
	 *
	 */
  void indexEntered(const QModelIndex &index);

  /* \brief Emits the activation signal for the current model index.
   *
   */
  void indexActivated();

signals:
  void activated(const QModelIndex &index);

private:
  QModelIndex m_rootModelIndex;
  QModelIndex m_currentModelIndex;
  QTreeView   m_treeView;
};


#endif // QCOMBOTREEVIEW_H
