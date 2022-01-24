#ifndef PMTABLEVIEW_H
#define PMTABLEVIEW_H

#include <QTableView>
#include <QObject>
#include <QKeyEvent>
#include "pmtablemodel.h"

class PMTableView : public QTableView
{
    Q_OBJECT

public:
    PMTableView();

protected:
    virtual void keyPressEvent(QKeyEvent *event); // method for clicking on row by Enter button

public slots:
    void showRows(bool starState); // show starred contacts
    void showRows(const QString& name, const QString& phone); // show rows suiting the specific requirements: name or/and phone number
    void showAllRows(); // show all rows
};

#endif // PMTABLEVIEW_H
