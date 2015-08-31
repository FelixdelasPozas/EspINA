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

#ifndef APP_TOOLGROUPS_FILE_FILESAVETOOL_H_
#define APP_TOOLGROUPS_FILE_FILESAVETOOL_H_

// ESPINA
#include <Support/Widgets/ProgressTool.h>

namespace ESPINA
{
  //----------------------------------------------------------------------------
  class FileSaveTool
  : public Support::Widgets::ProgressTool
  {
      Q_OBJECT
    public:
      /** \brief FileSaveTool class constructor.
       * \param[in] context application context.
       *
       */
      explicit FileSaveTool(Support::Context &context);

      /** \brief Sets the session filename.
       * \param[in] filename file to save the session.
       */
      void setSessionFile(const QString &filename);

    protected slots:
      /** \brief Saves the analysis to the given file.
       *
       */
      void save(const QString &fileName);

      /** \brief Zhu Li! Do the thing!!(tm)
       *
       */
      void onTriggered();

    private:
      QFileInfo m_sessionFile;
  };

  //----------------------------------------------------------------------------
  class FileSaveAsTool
  : public FileSaveTool
  {
      Q_OBJECT
    public:
      /** \brief FileSaveAsTool class constructor.
       * \param[in] context application context.
       *
       */
      explicit FileSaveAsTool(Support::Context &context);
  };

} // namespace ESPINA

#endif // APP_TOOLGROUPS_FILE_FILESAVETOOL_H_
