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

#ifndef APP_TOOLGROUPS_FILE_UNDOREDOTOOLS_H_
#define APP_TOOLGROUPS_FILE_UNDOREDOTOOLS_H_

// ESPINA
#include <Support/Context.h>
#include <Support/Widgets/ProgressTool.h>

namespace ESPINA
{
  /** \class UndoRedoTool
   * \brief Base class for undo/redo tools.
   *
   */
  class UndoRedoTool
  : public Support::Widgets::ProgressTool
  {
    Q_OBJECT
    public:
      /** \brief UndoRedoTool class constructor.
       * \param[in] context application context.
       * \param[in] id tool unique identificator.
       * \param[in] icon tool icon.
       * \param[in] tooltip tool initial tooltip.
       *
       */
      UndoRedoTool(Support::Context &context, const QString id, const QIcon icon, const QString tooltip);

      /** \brief UndoRedoTool class virtual destructor.
       *
       */
      virtual ~UndoRedoTool()
      {}

    signals:
      void executed();

    private slots:
      /** \brief Updates the tool state when the undo stack changes state.
       * \param[in] value true to enable the tool and false otherwise.
       *
       */
      void stateChanged(bool value);

      /** \brief Updates the tool tooltip when the undo stack changes state.
       * \param[in] text tooltip text.
       */
      void textChanged(const QString &text);

      /** \brief Executes the corresponding action.
       *
       */
      virtual void doAction() = 0;

    protected:
      QUndoStack *m_undoStack;

    private:
      QString m_tooltipPrefix; /** tool tooltip prefix (name of the tool). */
  };

  //----------------------------------------------------------------------------
  class UndoTool
  : public UndoRedoTool
  {
      Q_OBJECT
    public:
      /** \brief UndoTool class constructor.
       * \param[in] context application context.
       *
       */
      explicit UndoTool(Support::Context &context);

    private slots:
      virtual void doAction() override;
  };

  //----------------------------------------------------------------------------
  class RedoTool
  : public UndoRedoTool
  {
      Q_OBJECT
    public:
      /** \brief RedoTool class constructor.
       * \param[in] context application context.
       *
       */
      explicit RedoTool(Support::Context &context);

    private slots:
      virtual void doAction() override;
  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_FILE_UNDOREDOTOOLS_H_
