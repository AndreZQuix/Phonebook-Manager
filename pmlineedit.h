#ifndef PMLINEEDIT_H
#define PMLINEEDIT_H

#include <QLineEdit>
#include <QObject>
#include <QKeyEvent>

class PMLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    PMLineEdit(QWidget *parent);

protected:
    virtual void keyPressEvent(QKeyEvent *event) override; // overriden method for switching between widgets by keys

signals:
    void keyEnterPressed();
};

#endif // PMLINEEDIT_H
