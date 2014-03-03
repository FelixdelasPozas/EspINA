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

#include <App/Dialogs/TabularReport/TabularReport.h>
#include <ui_TabularReportEntry.h>

#include <GUI/Model/Proxies/InformationProxy.h>
#include <GUI/Widgets/InformationSelector.h>

#include <common/xlconfig.h>
#include <xlslib.h>
#include <QSettings>

namespace EspINA
{
  using ExcelWorkBook = std::shared_ptr<xlslib_core::workbook>;

  class TabularReport::Entry
  : public QWidget
  , public Ui_TabularReportEntry
  {
    Q_OBJECT
  public:
    explicit Entry(const QString   &category,
                   ModelAdapterSPtr model,
                   ModelFactorySPtr factory);
    virtual ~Entry();

    void setProxy(InformationProxy *proxy);

    int rowCount() const;

    int columnCount() const;

    QVariant value(int row, int column) const;


  private slots:
    void changeDisplayedInformation();

    void saveSelectedInformation();

    void extractInformation();

  private:
    bool exportToCSV(const QString &filename);

    bool exportToXLS(const QString &filename);

    InformationSelector::GroupedInfo availableInformation();

    QStringList lastInformationOrder();

    InformationSelector::GroupedInfo lastDisplayedInformation();

    void setInformation(InformationSelector::GroupedInfo extensionInformation, QStringList informationOrder);

    QStringList information(InformationSelector::GroupedInfo extensionInformation);

    QStringList updateInformationOrder(InformationSelector::GroupedInfo extensionInformation);

    void openCategorySettings(QSettings &settings);

    void closeCategorySettings(QSettings &settings);


  private:
    QString           m_category;
    ModelAdapterSPtr  m_model;
    ModelFactorySPtr  m_factory;

    InformationProxy *m_proxy;
  };
} // namespace EspINA

#endif // ESPINA_TABULAR_REPORT_ENTRY_H
