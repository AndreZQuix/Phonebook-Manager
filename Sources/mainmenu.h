#ifndef MAINMENU_H
#define MAINMENU_H

#include <QWidget>
#include <QLabel>
#include <QFileDialog>
#include "pmradiobutton.h"
#include "pmlineedit.h"
#include "pmpushbutton.h"
#include "pmtableview.h"
#include "pmtablemodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainMenu; }
QT_END_NAMESPACE

/* -------------------------------------------------------------------------------------------- */

/* this class is for data defining: get data and check it's correctness before sending to model */

/* -------------------------------------------------------------------------------------------- */

class MainMenu : public QWidget
{
    Q_OBJECT

public:
    MainMenu(QWidget *parent = nullptr);
    ~MainMenu();

private:
    Ui::MainMenu *ui;
    QSqlDatabase db;
    PMTableModel *model;
    PMTableView *view;
    QLabel *lblContactsCount;
    QString appName = "Phonebook Manager";

    void defineContact(bool needToChange, const QString& placeholderName = "Name",
                       const QString& placeholderPhoneNumber = "Phone number", const int groupID = 0,
                       const bool isStarred = false, const QModelIndex& index = QModelIndex()); // get and check data correctness and change it or add new contact to database
    void deleteContacts();  // delete selected contacts from database

private slots:
    void changeContactData(const QModelIndex& ind);  // get contact data and send it to defining method
    void searchContacts();  // find contact suiting specific requirements
    void changeGroup();     // ask model to change group name or delete it
    void exportContacts();  // get export settings and ask model to export contacts data
};
#endif // MAINMENU_H
