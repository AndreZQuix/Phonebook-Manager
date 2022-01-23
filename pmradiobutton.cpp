#include "pmradiobutton.h"

PMRadioButton::PMRadioButton(const QString& text, QWidget *parent, const QString& toolTipText) : QRadioButton(text, parent)
{
    setToolTip(toolTipText);
}

void PMRadioButton::keyPressEvent(QKeyEvent *event) // method for toggling the button by pressing Enter
{
    if(event->key() == Qt::Key_Return)
    {
        if(isChecked() == true)
        {
            setChecked(false);
            emit toggled(false);
        }
        else
        {
            setChecked(true);
            emit toggled(true);
        }
    }
    QRadioButton::keyPressEvent(event);
}
