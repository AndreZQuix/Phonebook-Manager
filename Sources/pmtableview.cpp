#include "pmtableview.h"
#include <QHeaderView>
#include <QAbstractItemView>

PMTableView::PMTableView()
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setFrameShape(QFrame::NoFrame);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void PMTableView::keyPressEvent(QKeyEvent *event) // method for clicking on row by Enter button
{
    if(event->key() == Qt::Key_Return)
        emit doubleClicked(currentIndex());
    QTableView::keyPressEvent(event);
}

void PMTableView::showRows(bool starState) // show starred contacts
{
    for(int row = 0; row < model()->rowCount(); row++)
    {
        if(model()->data(model()->index(row, PMTableModel::CONTACT_COLUMN::STAR)) == "-") // iterating through all rows to find unstarred contacts...
        {
            if(starState == true)   //...and hide it or show
                hideRow(row);
            else
                showRow(row);
        }
    }
}

void PMTableView::showAllRows() // show all rows
{
    for(int row = 0; row < model()->rowCount(); row++)
    {
        showRow(row);
    }
}

void PMTableView::showRows(const QString& name, const QString& phone) // show rows suiting the specific requirements: name or/and phone number
{
    if (!name.isEmpty() && !phone.isEmpty())    // search algorithm: iterating through all rows to find rows suiting specific requirements
    {
        for (int i = 0; i < model()->rowCount(); i++)
            if ((model()->data(model()->index(i, PMTableModel::CONTACT_COLUMN::NAME)) != name)
                    || (model()->data(model()->index(i, PMTableModel::CONTACT_COLUMN::PHONE)) != phone))
                hideRow(i);
    }
    else if (!name.isEmpty())
    {
        for (int i = 0; i < model()->rowCount(); i++)
            if(model()->data(model()->index(i, PMTableModel::CONTACT_COLUMN::NAME)) != name)
                hideRow(i);
    }
    else if (!phone.isEmpty())
    {
        for(int i = 0; i < model()->rowCount(); i++)
            if(model()->data(model()->index(i, PMTableModel::CONTACT_COLUMN::PHONE)) != phone)
                hideRow(i);
    }
}
