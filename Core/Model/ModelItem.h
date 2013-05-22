/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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


#ifndef MODELITEM_H
#define MODELITEM_H

#include <QModelIndex>

#include "Core/EspinaTypes.h"

#include <QStringList>

namespace EspINA
{
  class EspinaModel;
  class IEspinaModel;

  template<class T>
  QString arg3(const T val[3])
  {
    return QString("%1,%2,%3").arg(val[0]).arg(val[1]).arg(val[2]);
  }

  template<class T>
  QString arg6(const T val[6])
  {
    return QString("%1,%2,%3,%4,%5,%6").arg(val[0]).arg(val[1]).arg(val[2]).arg(val[3]).arg(val[4]).arg(val[5]);
  }

  typedef QList<ModelItemSPtr> ModelItemSList;

  struct Relation
  {
    ModelItemSPtr ancestor;
    ModelItemSPtr succesor;
    QString relation;
  };
  typedef QList<Relation> RelationList;

  const int RawPointerRole = Qt::UserRole+1;
  const int TypeRole       = Qt::UserRole+2;

  /// Base class for every item in EspinaModel
  class ModelItem
  : public QObject
  {
    Q_OBJECT
  public:
    typedef QString ArgumentId;
    typedef QString Argument;

    class Extension;
    typedef Extension * ExtensionPtr;

    typedef QString      ExtId;
    typedef QList<ExtId> ExtIdList;

    class Arguments
    : public QMap<ArgumentId, Argument>
    {
    public:
      explicit Arguments();
      explicit Arguments(const QMap<ArgumentId, Argument>& args);
      explicit Arguments(const QString args);
      virtual ~Arguments(){}

      virtual QString serialize() const;
      virtual QString hash() const;

    protected:
      inline QString argument(const QString &name, const QString &value) const
      { return QString("%1=%2;").arg(name).arg(value); }
    };

    static const ArgumentId EXTENSIONS;

    ModelItem() : m_model(NULL) {}
    virtual ~ModelItem();

    virtual QVariant data(int role=Qt::DisplayRole) const = 0;
    virtual bool setData(const QVariant& value, int role = Qt::UserRole +1) {return false;}
    virtual QString  serialize() const = 0;
    virtual ModelItemType type() const = 0;

    virtual void initialize(const Arguments &args = Arguments()) = 0;

//     /// Used to initialize its extension
//     /// It's important to call initialize once the item has stablished
//     /// its relations with other items. It's up to the developer to
//     /// satisfy this condition
//     virtual void initializeExtensions(const Arguments &args = Arguments()) = 0;

    ModelItemSList relatedItems(RelationType relType, const QString &relName = "");
    RelationList relations(const QString &relName = "");

  public slots:
    virtual void notifyModification()
    {emit modified(this);}

  signals:
    void modified(ModelItemPtr);

  protected:
    EspinaModel *m_model;

    friend class EspinaModel;
  };

  ModelItemPtr indexPtr(const QModelIndex &index);

} // EspINA

#endif //MODELITEM_H
