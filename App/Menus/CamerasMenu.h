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

#ifndef ESPINA_CAMERAS_MENU_H_
#define ESPINA_CAMERAS_MENU_H_

// ESPINA
#include <Support/ViewManager.h>
#include <GUI/View/RenderView.h>

// Qt
#include <QMenu>
#include <QList>

namespace ESPINA
{
  class CamerasMenu
  : public QMenu
  {
    Q_OBJECT
  public:
    explicit CamerasMenu(ViewManagerSPtr vm, QWidget *parent = 0);

    virtual ~CamerasMenu();

  private:
    /** \brief Struct to store all view's VisualState.
     *
     */
    struct CameraPositions
    {
      QString id;
      QList<struct RenderView::CameraState> states;
    };

    /** \brief List of CameraPositions.
     *
     */
    using CameraPositionsList = QList<struct CameraPositions>;

  public:
    /** \brief Loads Camera positions.
     *
     */
    void loadPositions(CameraPositionsList list);

  public slots:
    /** \brief Deletes stored camera positions.
     *
     */
    void clearPositions();

  private slots:
    /** \brief Activates the action passed as parameter.
     *
     */
    void activate(QAction *action);

  private:
    /** \brief Stores current VisualStates to a CameraPositionsList entry.
     *
     */
    void save();

  private:
    ViewManagerSPtr     m_viewManager;
    QAction            *m_save;
    QAction            *m_clear;
    QMenu              *m_load;
    CameraPositionsList m_cameraPositions;
  };

} // namespace ESPINA

#endif // ESPINA_VIEW_CAMERAS_MENU_H_
