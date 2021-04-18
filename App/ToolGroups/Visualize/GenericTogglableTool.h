/*
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_GENERIC_TOGGLABE_TOOL_H
#define ESPINA_GENERIC_TOGGLABE_TOOL_H

#include <Support/Widgets/ProgressTool.h>

namespace ESPINA
{
  /** \class GenericTogglableTool
   * \brief Implements a genering tool base class used for tooglable tools.
   *
   */
  class GenericTogglableTool
  : public Support::Widgets::ProgressTool
  {
    public:
      /** \brief GenericTogglableTool class constructor.
       * \param[in] id tool's id.
       * \param[in] icon tool's icon name from resources files.
       * \param[in] tooltip tool's tooltip.
       * \param[in] context application context.
       *
       */
      explicit GenericTogglableTool(const QString &id, const QString &icon, const QString &tooltip, Support::Context &context);

      /** \brief GenericTogglableTool class constructor.
       * \param[in] id tool's id.
       * \param[in] icon tool's icon.
       * \param[in] icon tool's icon name from resources files.
       * \param[in] tooltip tool's tooltip.
       * \param[in] context application context.
       *
       */
      explicit GenericTogglableTool(const QString &id, const QIcon &icon, const QString &tooltip, Support::Context &context);

      /** \brief GenericTogglableTool class virtual destructor.
       *
       */
      virtual ~GenericTogglableTool()
      {}

      virtual void saveSettings(std::shared_ptr<QSettings> settings) override;

      virtual void restoreSettings(std::shared_ptr< QSettings > settings) override;
  };
}

#endif // ESPINA_GENERIC_TOGGLABE_TOOL_H
