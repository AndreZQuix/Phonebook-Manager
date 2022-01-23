#include "pmpushbutton.h"

PMPushButton::PMPushButton(const QString& name, QWidget *parent, const QString& toolTipText) : QPushButton(name, parent)
{
    setToolTip(toolTipText);
}

void PMPushButton::keyPressEvent(QKeyEvent *event) // overriden method for switching between widgets by keys and clicking the button by Enter
{
    switch(event->key())
    {
    case Qt::Key_Return:
        emit clicked();
        break;

    case Qt::Key_Down:
        focusNextChild();
        break;

    case Qt::Key_Up:
        focusPreviousChild();
        break;

    default:
        QPushButton::keyPressEvent(event);
        break;
    }
}
