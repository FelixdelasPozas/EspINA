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

#ifndef ESPINA_POSITION_MARKS_TOOL_H
#define ESPINA_POSITION_MARKS_TOOL_H

// ESPINA
#include <GUI/View/RenderView.h>
#include <Support/Widgets/ProgressTool.h>

class QComboBox;
class QPushButton;

namespace ESPINA
{
  /** \class PositionMarksTool
   * \brief Tool button for the position marks.
   *
   */
  class PositionMarksTool
  : public Support::Widgets::ProgressTool
  {
      Q_OBJECT
    public:
      /** \brief PositionMarksTool class constructor.
       * \param[in] context espina context reference.
       * \param[in] views list of views to get/set visual bookmarks.
       *
       */
      PositionMarksTool(Support::Context &context, QList<RenderView *> views);

      /** \brief VisualBookmarls class virtual destructor.
       *
       */
      virtual ~PositionMarksTool();

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
        QString id;                                            /** position identificator.                */
        QMap<QString, struct RenderView::CameraState> states;  /** map of view<->state for this position. */
      };

      using CameraPositionsList = QList<struct CameraPositions>;

      QList<RenderView *> m_views;     /** list of views of the application that will be stored in a position. */
      CameraPositionsList m_bookmarks; /** list of stored camera positions.                                    */

      QPushButton *m_add;      /** add position button.                         */
      QPushButton *m_remove;   /** remove position button.                      */
      QPushButton *m_apply;    /** apply selected position button.              */
      QComboBox   *m_combobox; /** combo box for selection of stored positions. */
  };

} // namespace ESPINA

#endif // ESPINA_POSITION_MARKS_TOOL_H
