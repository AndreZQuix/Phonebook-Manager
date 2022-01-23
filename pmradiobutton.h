#ifndef PMRADIOBUTTON_H
#define PMRADIOBUTTON_H

#include <QRadioButton>
#include <QObject>
#include <QKeyEvent>

class PMRadioButton : public QRadioButton
{
    Q_OBJECT

public:
    PMRadioButton(const QString& text, QWidget *parent, const QString& toolTipText);

protected:
    virtual void keyPressEvent(QKeyEvent *event); // method for toggling the button by pressing Enter
};

#endif // PMRADIOBUTTON_H
