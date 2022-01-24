#include "pmtablemodel.h"
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlRelation>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextTableCell>
#include <QPrinter>

PMTableModel::PMTableModel(QObject *parent, QSqlDatabase db) : QSqlRelationalTableModel(parent, db)
{
    db.setDatabaseName("./ContactDB.db");   // open connection with database...
    if(db.open())
    {
        qDebug("DB is opened");
    }
    else
    {
        qDebug("DB cannot be opened");
    }

    query = new QSqlQuery(db);      // ...create two new tables, set references...
    if(query->exec("CREATE TABLE "
                   "ContactBook(id INTEGER PRIMARY KEY AUTOINCREMENT, Name TEXT, PhoneNumber TEXT, GroupID INT, IsStarred TEXT, FOREIGN KEY (GroupID) REFERENCES Groups(id))"))
    {
        qDebug("ContactBook has been successfully created");
    }
    else
    {
        qDebug() << query->lastError().text();  // ... and insert one new row: "no group" parameter
    }
    if(query->exec("CREATE TABLE "
                   "Groups(id INTEGER PRIMARY KEY AUTOINCREMENT, GroupName TEXT)") && query->exec("INSERT INTO Groups (GroupName) VALUES ('')"))
    {
        qDebug("GroupsTable has been successfully created");
    }
    else
    {
        qDebug("GroupsTable Error");
    }

    this->db = db;
}

std::tuple<int, QString, QString, int, bool> PMTableModel::getContactData(const QModelIndex& ind) // get all contact data
{
    int row = ind.row();
    int id = data(index(row, ID)).toInt();
    query->prepare("SELECT * FROM ContactBook WHERE id = :id"); // get values from database and return a tuple
    query->bindValue(":id", id);
    query->exec();
    query->first();

    QString name = query->value(NAME).toString();
    QString phone = query->value(PHONE).toString();
    int group = query->value(GR_ID).toInt();
    bool star = false;
    query->value(STAR).toString() == "+" ? star = true : star;
    qDebug() << query->value(0).toString() << query->value(1).toString() << query->value(2).toString();
    return std::make_tuple(id, name, phone, group, star);
}

void PMTableModel::addContacts(const QString& name, const QString& phoneNumber, const int groupID, const bool isStarred) // add contact to database
{
    query->prepare("INSERT INTO ContactBook (Name, PhoneNumber, GroupID, IsStarred) "
                   "VALUES (:name, :phone, :groupID, :star)");
    query->bindValue(":name", name);
    query->bindValue(":groupID", groupID);
    query->bindValue(":phone", phoneNumber);
    isStarred == true ? query->bindValue(":star", "+") : query->bindValue(":star", "-");
    if(query->exec()) qDebug() << "Contact has been successful added"; else qDebug() << "Contact adding error: " << query->lastError().text();
    submitAll();
    select();
}

void PMTableModel::changeData(const QModelIndex& ind, const QString& name, const QString& phoneNumber, const int groupID, const bool isStarred) // change contact data
{
    int row = ind.row();
    int id = data(index(row, ID)).toInt();
    query->prepare("UPDATE ContactBook SET Name = :name, PhoneNumber = :phone, GroupID = :groupID, IsStarred = :star WHERE id = :id");
    query->bindValue(":name", name);
    query->bindValue(":phone", phoneNumber);
    query->bindValue(":groupID", groupID);
    isStarred == true ? query->bindValue(":star", "+") : query->bindValue(":star", "-");
    query->bindValue(":id", id);
    query->exec();
    submitAll();
    select();
}

void PMTableModel::deleteContacts(QModelIndexList& indexes) // delete contact(s) from database
{
    std::sort(indexes.begin(), indexes.end(), std::less<QModelIndex>());    // sort selected rows by index...
    int countRow = indexes.count();

    for (int i = countRow; i > 0; i--)  // ...and delete it one by one
    {
        deleteRowFromTable(indexes.at(i-1).row());
    }
    submitAll();
    select();
}

int PMTableModel::addGroup(const QString& groupName) // add group to database
{
    query->prepare("INSERT INTO Groups (GroupName) VALUES (:groupName)");   // insert group to database...
    query->bindValue(":groupName", groupName);
    if(query->exec()) qDebug("Insertion successed"); else qDebug() << "AddGroup Error " << query->lastError().text();
    int id = query->lastInsertId().toInt();
    qDebug() << id;
    return id;  // ...and return id
}

QMap<int, QString> PMTableModel::getGroups() // get all groups from database: group id and it's name
{
    QMap<int, QString> groupMap;    // get all groups from data base...
    int fieldNo, id;
    if(query->exec("SELECT id, GroupName FROM Groups")) qDebug("Selection successed heHHHHHH"); else qDebug() << query->lastError().text();
    while(query->next())
    {
        fieldNo = query->record().indexOf("GroupName");     // ...and add to map: id and it's name
        id = query->record().indexOf("id");
        qDebug() << query->value(id).toInt();
        qDebug() << query->value(fieldNo).toString();
        groupMap.insert(query->value(id).toInt(), query->value(fieldNo).toString());
    }
    return groupMap;
}

QString PMTableModel::getGroupName(const int groupID) // get group name by id
{
    query->prepare("SELECT GroupName FROM Groups WHERE id = :groupID"); // find group by unique id...
    query->bindValue(":groupID", groupID);
    if(query->exec()) qDebug("Selection group successed"); else qDebug() << query->lastError().text();
    query->first();
    return query->value(0).toString();  // ...and return it's name
}

int PMTableModel::getGroupSize(const int groupID) // get group size by id
{
    query->prepare("SELECT COUNT(*) FROM ContactBook WHERE GroupID = :groupID");  // count rows that groupID suiting the specific id...
    query->bindValue(":groupID", groupID);
    query->exec();
    query->first();
    return query->value(0).toInt(); // ...and return the number
}

void PMTableModel::deleteGroup(const int groupID) // delete group from database without contacts deleting
{
    query->prepare("UPDATE ContactBook SET GroupID = 1 WHERE GroupID = :groupID");  // groupID = 1 is a "no group" parameter
    query->bindValue(":groupID", groupID);  // update contacts with specific groupID: set groupID to "no group parameter"...
    query->exec();
    query->prepare("DELETE FROM Groups WHERE id = :groupID");   // ...find and delete group with specific id
    query->bindValue(":groupID", groupID);
    if(query->exec()) qDebug("Delete group successed"); else qDebug() << query->lastError().text();
    select();
}

void PMTableModel::changeGroupName(const QString newName, const int groupID) // set new group name
{
    query->prepare("UPDATE Groups SET GroupName = :newName WHERE id = :groupID");   // update groups with specific id with new name
    query->bindValue(":newName", newName);
    query->bindValue(":groupID", groupID);
    if(query->exec()) qDebug("Name changed successfully"); else qDebug() << "Name changing error " << query->lastError().text() << groupID;
    select();
}

void PMTableModel::exportContactsPDF(const QString& filePath, const bool byGroup, const int groupID) // export contacts as PDF-file by specific requirements
{
    QTextDocument *doc = new QTextDocument(this); // create document and style
    doc->setDocumentMargin(10);
    QTextCursor cursor(doc);

    cursor.movePosition(QTextCursor::Start);

    QTextTableFormat format;
    format.setAlignment(Qt::AlignVCenter);
    format.setCellPadding(10.0);
    format.setBorderStyle(QTextFrameFormat::BorderStyle::BorderStyle_Ridge);

    QTextCharFormat textFormat;
    textFormat.setFontPointSize(14.0);

    int columnCount;      // calculate the row and column count of table
    int exportedRowCount;
    if(byGroup)
    {
        columnCount = this->columnCount() - 3; // no need to export next columns: ID, GroupID, isStarred
        exportedRowCount = getGroupSize(groupID);   // only contacts with specific groupID are exporting
    }
    else
    {
        columnCount = this->columnCount() - 2; // no need to export next columns: ID, isStarred
        exportedRowCount = rowCount();
    }

    QTextTable *table = cursor.insertTable(exportedRowCount + 1, columnCount);  // columns creating
    table->setFormat(format);
    QTextTableCell headerCell(table->cellAt(0,0));
    headerCell.setFormat(textFormat);
    QTextCursor headerCellCursor = headerCell.firstCursorPosition();
    headerCellCursor.insertText("Name");

    headerCell = table->cellAt(0,1);
    headerCell.setFormat(textFormat);
    headerCellCursor = headerCell.firstCursorPosition();
    headerCellCursor.insertText("Phone number");

    if(!byGroup)
    {
        headerCell = table->cellAt(0,2);
        headerCell.setFormat(textFormat);
        headerCellCursor = headerCell.firstCursorPosition();
        headerCellCursor.insertText("Group");
    }

    QTextTableCell cell;
    QTextCursor cellCursor;
    int row = 1;
    if(byGroup)     // if export by group...
    {
        query->prepare("SELECT Name, PhoneNumber FROM ContactBook WHERE GroupID = :groupID");   // ...select contacts with specific groupID
        query->bindValue(":groupID", groupID);
        query->exec() == true ? qDebug() << "by group success" : qDebug() << "no success by group";
        while(query->next())    // fill row with data: the first one is contact name, the second one is contact phone number
        {
            cell = table->cellAt(row, 0);
            cell.setFormat(textFormat);
            cellCursor = cell.firstCursorPosition();
            cellCursor.insertText(query->value(0).toString());

            cell = table->cellAt(row, 1);
            cell.setFormat(textFormat);
            cellCursor = cell.firstCursorPosition();
            cellCursor.insertText(query->value(1).toString());

            row++;  // go to the next row
        }
    }
    else    // if export all contacts
    {
        for(row = 0; row < exportedRowCount; row++) // fill the table with the data: contact name, phone number and group name
            for(int column = 0; column < columnCount; column++)
            {
                cell = table->cellAt(row + 1, column);
                cell.setFormat(textFormat);
                cellCursor = cell.firstCursorPosition();

                cellCursor.insertText(data(index(row, column + 1)).toString());
            }
    }

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath + "/Exported contacts.pdf");
    doc->print(&printer);
}
