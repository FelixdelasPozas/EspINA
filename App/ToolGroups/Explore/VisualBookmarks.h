/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_VISUAL_BOOKMARKS_H_
#define ESPINA_VISUAL_BOOKMARKS_H_

// ESPINA
#include <GUI/View/RenderView.h>
#include <Support/Widgets/ProgressTool.h>

using namespace ESPINA::Support::Widgets;

class QComboBox;
class QPushButton;

namespace ESPINA
{
  class VisualBookmarks
  : public ProgressTool
  {
      Q_OBJECT
    public:
      /** \brief VisualBookmarks class constructor.
       * \param[in] context espina context reference.
       * \param[in] views list of views to get/set visual bookmarks.
       *
       */
      VisualBookmarks(Support::Context &context, QList<RenderView *> views);

      /** \brief VisualBookmarls class virtual destructor.
       *
       */
      virtual ~VisualBookmarks();

      virtual void abortOperation() override;

      virtual void saveSettings(std::shared_ptr<QSettings> settings) override final;

      virtual void restoreSettings(std::shared_ptr<QSettings> settings) override final;

    public slots:
      /** \brief Clears the internal data and the resets the gui.
       *
       */
      void clear();

    private slots:
      /** \brief Adds a new bookmark to the list.
       *
       */
      void add();

      /** \brief Removes current bookmark.
       *
       */
      void remove();

      /** \brief Applies current bookmark.
       *
       */
      void apply();

    private:
      /** \brief Helper method to initialize the additional widgets.
       *
       */
      void initWidgets();

      /** \brief Resets the combo box to the initial state.
       *
       */
      void resetComboBox();

      /** \brief Struct to store all view's states.
       *
       */
      struct CameraPositions
      {
        QString id;
        QMap<QString, struct RenderView::CameraState> states;
      };

      using CameraPositionsList = QList<struct CameraPositions>;

      QList<RenderView *> m_views;
      CameraPositionsList m_bookmarks;

      QPushButton *m_add;
      QPushButton *m_remove;
      QPushButton *m_apply;
      QComboBox   *m_combobox;
  };

} // namespace ESPINA

#endif // ESPINA_VISUAL_BOOKMARKS_H_
