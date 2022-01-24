#ifndef PMTABLEMODEL_H
#define PMTABLEMODEL_H

#include <QSqlTableModel>
#include <QObject>
#include <QFile>
#include <QDir>
#include <QSqlDatabase>
#include <QDebug>
#include <QSqlRelationalTableModel>
#include <QSqlQuery>
#include <QMap>
#include <tuple>

class PMTableModel : public QSqlRelationalTableModel
{
    Q_OBJECT

    QSqlQuery *query;
    QSqlDatabase db;

public:
    enum CONTACT_COLUMN{
        ID, NAME, PHONE, GR_ID, STAR
    };

    enum GROUP_COLUMN{
        GROUP_ID, GROUP_NAME
    };

    PMTableModel(QObject *parent, QSqlDatabase db);

    std::tuple<int, QString, QString, int, bool> getContactData(const QModelIndex& ind);    // get all contact data
    void addContacts(const QString& name, const QString& phoneNumber, const int groupID, const bool isStarred); // add contact to database
    void changeData(const QModelIndex& index, const QString& name, const QString& phoneNumber, const int groupID, const bool isStarred);    // change contact data
    void deleteContacts(QModelIndexList& indexes);  // delete contact(s) from database
    int addGroup(const QString& groupName); // add group to database
    QMap<int, QString> getGroups(); // get all groups from database: group id and it's name
    QString getGroupName(const int groupID);    // get group name by id
    int getGroupSize(const int groupID);    // get group size by id
    void deleteGroup(const int groupID);    // delete group from database without contacts deleting
    void changeGroupName(const QString newName, const int groupID); // set new group name
    void exportContactsPDF(const QString& filePath, const bool byGroup = false, const int groupID = 0); // export contacts as PDF-file by specific requirements
};

#endif // PMTABLEMODEL_H
