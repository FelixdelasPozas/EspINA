/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
 *
 *    This program is free software: you can redistribute it and/or modify
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


#ifndef VISUALIZATIONSTATE_H
#define VISUALIZATIONSTATE_H

#include "EspinaGUI_Export.h"

#include "Core/Extensions/SegmentationExtension.h"
#include "Core/Extensions/ModelItemExtension.h"
#include "Core/EspinaTypes.h"

// ITK
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkStatisticsLabelObject.h>

namespace EspINA
{
  const ModelItem::ExtId VisualizationStateID = "VisualizationExtension";

  class EspinaGUI_EXPORT VisualizationState
  : public Segmentation::Information
  {
    struct EspinaGUI_EXPORT ExtensionData
    {
      ExtensionData();

      bool operator==(const ExtensionData& other) const
      {
        return (Settings == other.Settings);
      }

      QMap<QString, QString> Settings;
    };

    typedef Cache<SegmentationPtr, ExtensionData> ExtensionCache;

    static ExtensionCache s_cache;

    const static QString EXTENSION_FILE;

  public:
    explicit VisualizationState();
    virtual ~VisualizationState();

    virtual ModelItem::ExtId id();

    virtual ModelItem::ExtIdList dependencies() const
    { return Segmentation::Extension::dependencies(); }

    virtual Segmentation::InfoTagList availableInformations() const;

    virtual bool validTaxonomy(const QString &qualifiedName) const
    { return true;}

    virtual void setSegmentation(SegmentationPtr seg);

    virtual QVariant information(const Segmentation::InfoTag &tag);

    virtual bool isCacheFile(const QString &file) const
    { return EXTENSION_FILE == file; }

    virtual void loadCache(QuaZipFile  &file,
                           const QDir  &tmpDir,
                           IEspinaModel *model);

    virtual bool saveCache(Snapshot &snapshot);

    virtual Segmentation::InformationExtension clone();

    virtual void initialize();

    virtual void invalidate(SegmentationPtr segmentation = NULL);

    void setSettings(QString key, QString settings);

    QString settings(QString key);
  };

}// namespace EspINA

#endif // VISUALIZATIONSTATE_H
