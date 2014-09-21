/*
    Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TAGSELECTOR_H
#define TAGSELECTOR_H

#include "GUI/EspinaGUI_Export.h"

// Qt
#include <QDialog>
#include <QStandardItemModel>

namespace ESPINA
{
  class EspinaGUI_EXPORT TagSelector
  : public QDialog
  {
    Q_OBJECT

    class GUI;

  public:
    /** brief TagSelector class constructor.
     * \param[in] title, title of the dialog.
     * \param[in] tags, list of tags.
     * \param[in] parent, raw pointer of the QWidget parent of this one.
     * \param[in] flags, window flags.
     */
    explicit TagSelector(const QString &title,
                         QStandardItemModel &tags,
                         QWidget *parent = nullptr,
                         Qt::WindowFlags flags = 0);

    /** brief TagSelector class virtual destructor.
     *
     */
    virtual ~TagSelector();

  private slots:
		/** brief Creates a new tag element from the user input.
		 *
		 */
    void createTag();

  private:
    GUI *m_gui;
    QStandardItemModel &m_tags;
  };
} // namespace ESPINA

#endif // TAGSELECTOR_H
