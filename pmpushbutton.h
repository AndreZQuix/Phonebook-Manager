#ifndef PMPUSHBUTTON_H
#define PMPUSHBUTTON_H

#include <QPushButton>
#include <QObject>
#include <QKeyEvent>

class PMPushButton : public QPushButton
{
    Q_OBJECT

public:
    PMPushButton(const QString& name, QWidget *parent, const QString& toolTipText);

protected:
    virtual void keyPressEvent(QKeyEvent *event) override; // overriden method for switching between widgets by keys and clicking the button by Enter
};

#endif // PMPUSHBUTTON_H

