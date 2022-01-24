#include "pmlineedit.h"

PMLineEdit::PMLineEdit(QWidget *parent) : QLineEdit(parent) {}

void PMLineEdit::keyPressEvent(QKeyEvent *event) // overriden method for switching between widgets by keys
{
    switch(event->key())
    {
    case Qt::Key_Return:
    case Qt::Key_Down:
        focusNextChild();
        break;

    case Qt::Key_Up:
        focusPreviousChild();
        break;

    default:
        QLineEdit::keyPressEvent(event);
        break;
    }
}
