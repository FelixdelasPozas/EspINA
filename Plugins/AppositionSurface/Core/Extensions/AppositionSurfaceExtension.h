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
#ifndef APPOSITIONSURFACEXTENSION_H
#define APPOSITIONSURFACEXTENSION_H

// EspINA
#include <Core/Extensions/SegmentationExtension.h>
#include <Core/EspinaTypes.h>

namespace EspINA
{
  const QString SAS = QObject::tr("SAS");

  class AppositionSurfaceExtension
  : public Segmentation::Information
  {
      struct CacheEntry
      {
          explicit CacheEntry();

          bool Modified;
          double Area;
          double Perimeter;
          double Tortuosity;
          QString FromSynapse;
      };

      static QMap<SegmentationPtr, CacheEntry> s_cache;

    public:
      static const ModelItem::ExtId ID;

      static const Segmentation::InfoTag AREA;
      static const Segmentation::InfoTag PERIMETER;
      static const Segmentation::InfoTag TORTUOSITY;
      static const Segmentation::InfoTag SYNAPSE;

      const static QString EXTENSION_FILE;

    public:
      explicit AppositionSurfaceExtension();
      virtual ~AppositionSurfaceExtension();

      virtual ModelItem::ExtId id();

      virtual ModelItem::ExtIdList dependencies() const
      {
        return Segmentation::Extension::dependencies();
      }

      virtual Segmentation::InfoTagList availableInformations() const;

      virtual QVariant information(const Segmentation::InfoTag &tag);

      virtual void initialize(ModelItem::Arguments args = ModelItem::Arguments());

      virtual bool isCacheFile(const QString &file) const;

      virtual bool loadCache(QuaZipFile &file, const QDir &tmpDir, EspinaModel *model);

      virtual bool saveCache(Snapshot &cacheList);

      virtual Segmentation::InformationExtension clone();

    private:
      virtual void invalidate();

  };

  typedef QSharedPointer<AppositionSurfaceExtension> AppositionSurfaceExtensionSPtr;

} // namespace EspINA

#endif // APPOSITIONSURFACEEXTENSION_H
