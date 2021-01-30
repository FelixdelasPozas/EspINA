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

#ifndef ESPINA_TABULAR_REPORT_ENTRY_H
#define ESPINA_TABULAR_REPORT_ENTRY_H

#include "Support/EspinaSupport_Export.h"

//ESPINA
#include "TabularReport.h"
#include <ui_TabularReportEntry.h>
#include <GUI/Model/Proxies/InformationProxy.h>
#include <GUI/Widgets/InformationSelector.h>

// xlslib
#include <common/xlconfig.h>
#include <xlslib.h>

// Qt
#include <QSettings>

namespace ESPINA
{
  /** \class TabularReport::Entry
   * \brief Individual tab entry for TabularReport widget.
   *
   */
  class EspinaSupport_EXPORT TabularReport::Entry
  : public QWidget
  , public Ui_TabularReportEntry
  {
    Q_OBJECT
  public:
    /** \brief Entry class constructor.
     * \param[in] category name of the tab's category.
     * \param[in] model session model.
     * \param[in] factory model factory.
     * \param[in] parent pointer to the widget parent of this one.
     *
     */
    explicit Entry(const QString   &category,
                   ModelAdapterSPtr model,
                   ModelFactorySPtr factory,
                   QWidget         *parent);

    /** \brief Entry class virtual destructor.
     *
     */
    virtual ~Entry();

    /** \brief Sets the information proxy for the model of the entry.
     *
     */
    void setProxy(InformationProxy *proxy);

    /** \brief Returns the number of columns of the table.
     *
     */
    virtual int rowCount() const;

    /** \brief Returns the number of rows of the table.
     *
     */
    virtual int columnCount() const;

    /** \brief Returns the contents the the given table position.
     * \param[in] row row index.
     * \param[in] column column index.
     *
     */
    virtual QVariant value(int row, int column) const;

  signals:
    void informationReadyChanged();

  protected:
    virtual void paintEvent(QPaintEvent* event);

    /** \brief Returns the available information to be selected to show on the table.
     *
     */
    virtual GUI::InformationSelector::GroupedInfo availableInformation();

    /** \brief Saves the contents of the table to the given file on disk on CSV format.
     * \param[in] filename file name.
     *
     */
    void exportToCSV(const QString &filename);

    /** \brief Saves the contents of the table to the fiven file on disk on XLS format.
     * \param[in] filename file name.
     *
     */
    void exportToXLS(const QString &filename);

  protected slots:
    /** \brief Exports all table information to disk.
     *
     */
    virtual void extractInformation();

  private slots:
    /** \brief Launches the information selection dialog and modifies the contents on the table with the selected fields.
     *
     */
    virtual void changeDisplayedInformation();

    /** \brief Saves the fields of the table to the session temporal directory.
     *
     */
    virtual void saveSelectedInformation();

    /** \brief Refreshes the table contents when changed.
     *
     */
    virtual void refreshAllInformation();

    /** \brief Invalidates all shown information and forces a refresh.
     *
     */
    virtual void forceRefreshAllInformation();

    /** \brief Refreshes the entry GUI.
     *
     */
    void refreshGUI();

  private:
    /** \brief Private implementation of the GUI refresh.
     *
     */
    virtual void refreshGUIImplementation();

    /** \brief Returns the name of the file that contains the information fields to show in the table.
     *
     */
    virtual QString selectedInformationFile() const
    {
      QString path = m_category;

      return TabularReport::extraPath(path.replace("/","_") + ".txt");
    }

    /** \brief Returns the old versions' filename for this entry.
     *
     * NOTE: not compatible with windows OS
     *
     */
    virtual const QString oldSelectedInformationFile() const
    {
      QString path = m_category;

      return TabularReport::extraPath(path.replace("/",">") + ".txt");
    }

    /** \brief Helper method to return the saved information fields (last used).
     *
     */
    virtual Core::SegmentationExtension::InformationKeyList lastInformationOrder();

    /** \brief Helper method to return the saved information fields (last used) in GroupedInfo format.
     *
     */
    virtual GUI::InformationSelector::GroupedInfo lastDisplayedInformation();

    /** \brief Sets the new information fields to show on the table.
     * \param[in] extensionInformation information provided by extensions.
     * \param[in] informationOrder order or the fields to show.
     *
     */
    virtual void setInformation(GUI::InformationSelector::GroupedInfo extensionInformation, Core::SegmentationExtension::InformationKeyList informationOrder);

    /** \brief Returns the information provided by the given extensions.
     * \param[in] extensionInformation extensions information key list.
     *
     */
    Core::SegmentationExtension::InformationKeyList information(GUI::InformationSelector::GroupedInfo extensionInformation);

    /** \brief Updates the fields order.
     * \param[in] extensionInformation extensions information key list.
     *
     */
    Core::SegmentationExtension::InformationKeyList updateInformationOrder(GUI::InformationSelector::GroupedInfo extensionInformation);

  protected:
    QString           m_category; /** category of the entry.         */
    ModelAdapterSPtr  m_model;    /** session model.                 */
    ModelFactorySPtr  m_factory;  /** model factory.                 */
    InformationProxy *m_proxy;    /** entry model information proxy. */
  };
} // namespace ESPINA

#endif // ESPINA_TABULAR_REPORT_ENTRY_H
