#include "mainmenu.h"
#include "ui_mainmenu.h"
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QComboBox>
#include <QMenuBar>

/* -------------------------------------------------------------------------------------------- */

/* this class is for data defining: get data and check it's correctness before sending to model */

/* -------------------------------------------------------------------------------------------- */

MainMenu::MainMenu(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainMenu)
{
    ui->setupUi(this);

    db = QSqlDatabase::addDatabase("QSQLITE");
    model = new PMTableModel(this, db);
    model->setTable("ContactBook");
    model->setRelation(PMTableModel::CONTACT_COLUMN::GR_ID, QSqlRelation("Groups", "id", "GroupName"));
    model->select();

    view = new PMTableView;
    view->setModel(model);
    view->setColumnHidden(PMTableModel::CONTACT_COLUMN::ID, true);
    connect(view, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(changeContactData(const QModelIndex&)));

    QGridLayout *grid = new QGridLayout(this);

    PMRadioButton *byStar = new PMRadioButton("&Show only starred", this, "Show only starred contacts");
    connect(byStar, SIGNAL(toggled(bool)), view, SLOT(showRows(bool)));

    PMPushButton *addContact = new PMPushButton("&Add contacts", this, "Add new contact to the contact storage");
    connect(addContact, &PMPushButton::clicked, this, [this] { defineContact(false); });

    PMPushButton *showContacts = new PMPushButton("Show all contacts", this, "Show all contacts");
    connect(showContacts, &PMPushButton::clicked, this, [this, byStar] {
        view->showAllRows();
        byStar->setChecked(false);
    });

    PMPushButton *searchContacts = new PMPushButton("Search contacts", this, "Find contacts by parameters");
    connect(searchContacts, &PMPushButton::clicked, this, &MainMenu::searchContacts);

    PMPushButton *changeGroup = new PMPushButton("Change group", this, "Change group parameters");
    connect(changeGroup, &PMPushButton::clicked, this, &MainMenu::changeGroup);

    PMPushButton *deleteContacts = new PMPushButton("Delete contacts", this, "Select contacts to delete and press this button");
    connect(deleteContacts, &PMPushButton::clicked, this, &MainMenu::deleteContacts);

    PMPushButton *exportContacts = new PMPushButton("Export contacts", this, "Export contacts from the contactbook");
    connect(exportContacts, &PMPushButton::clicked, this, &MainMenu::exportContacts);

    QHBoxLayout *boxLayout = new QHBoxLayout(this);
    QVector<PMPushButton*> buttonVec = QVector<PMPushButton*>{addContact, showContacts, searchContacts, changeGroup, deleteContacts, exportContacts};

    QMenuBar *menuBar = new QMenuBar(this);
    grid->addWidget(menuBar);
    QMenu *about = menuBar->addMenu("&Help");
    QAction *aboutProgram = new QAction("&About app", this);
    about->addAction(aboutProgram);
    connect(aboutProgram, &QAction::triggered, this, [this] { QMessageBox::information(this, appName, "Version 1.0.0\nDeveloper: https://github.com/AndreZQuix"); });

    for(const auto& button : buttonVec)
        boxLayout->addWidget(button);
    grid->addLayout(boxLayout, 1, 0);

    grid->addWidget(byStar);

    lblContactsCount = new QLabel("There are " + QString::number(model->rowCount()) + " contact(s) in the phonebook", this);
    grid->addWidget(lblContactsCount);

    grid->addWidget(view);
    setLayout(grid);
    setWindowTitle(appName);
}

MainMenu::~MainMenu()
{
    delete ui;
}

/* -------------------------------------------------------------------------------------------- */

/*                         defineContact() has 2 operation modes:                               */
/*                  1. Add contact to model (boolean needToChange is false)                     */
/*                   2. Change contact data (boolean needToChange is true)                      */

/* -------------------------------------------------------------------------------------------- */

void MainMenu::defineContact(bool needToChange, const QString& placeholderName, const QString& placeholderPhoneNumber, const int groupID, const bool isStarred, const QModelIndex& index)
{
    QWidget *wdg = new QWidget();
    wdg->setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    wdg->setWindowTitle(appName);
    wdg->setMinimumWidth(400);

    QGridLayout *grid = new QGridLayout(this);
    QHBoxLayout *boxLayout = new QHBoxLayout(this);

    PMLineEdit *edtName = new PMLineEdit(this);
    edtName->setPlaceholderText(placeholderName);

    PMLineEdit *edtNum = new PMLineEdit(this);
    edtNum->setPlaceholderText(placeholderPhoneNumber);
    QRegularExpression rx("\\+?\\d{1,4}?(\\s?|.?)\\d{3}(\\s?|.?)\\d{3}(\\s?|.?)\\d{2}(\\s?|.?)\\d{2}");
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(rx, this);
    edtNum->setValidator(validator);

    QComboBox *groupBox = new QComboBox(this);  // combo box with group list
    QMap<int, QString> map = model->getGroups();
    QMap<int, QString>::iterator it;
    for(it = map.begin(); it != map.end(); it++)
    {
        if(it.key() == 1) groupBox->addItem("No group", it.key());  // groupID = 1 is a "no group" parameter
            else groupBox->addItem(it.value(), it.key());
    }

    PMRadioButton *starContact = new PMRadioButton("&Star contact", this, "Add this contact to important");
    starContact->setChecked(isStarred);

    PMPushButton *addGroup = new PMPushButton("&Create new group", this, " ");
    PMLineEdit *edtGroup  = new PMLineEdit(this);
    connect(addGroup, &PMPushButton::clicked, this, [=] // if "Create new group" button was clicked...
    {
        edtGroup->setPlaceholderText("Type a new group name. If you changed your mind, leave this field empty");    // ...add new line edit (field) instead of this button
        grid->removeWidget(addGroup);
        addGroup->setVisible(false);
        boxLayout->addWidget(edtGroup);
    });

    QDialogButtonBox *defineContactBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    if(needToChange)    // if need to change the contact...
    {
        edtName->setText(placeholderName);  // ...set it's data as text in fields
        edtNum->setText(placeholderPhoneNumber);
        starContact->setChecked(isStarred);
        groupBox->setCurrentText(model->getGroupName(groupID));
    }

    connect(defineContactBox, &QDialogButtonBox::accepted, this, [=]() mutable
    {
        int id;
        if(!edtGroup->text().simplified().isEmpty())    // if new group field is not empty...
        {
            id = model->addGroup(edtGroup->text().simplified());    // ...add new group and get it's id...
        } else
        {
            id = groupBox->currentData().toInt();   // ...else get current group id from combo box
        }

        if (!edtName->text().simplified().isEmpty() && (!edtNum->text().simplified().isEmpty()))    // if name andd phone number fields are not empty...
        {
            if(!needToChange)
            {
                model->addContacts(edtName->text().simplified(), edtNum->text().simplified(), id, starContact->isChecked());    // add new contact to the database
                lblContactsCount->setText("There are " + QString::number(model->rowCount()) + " contact(s) in the phonebook");  // update the contacts counter
            } else
            {
                model->changeData(index, edtName->text().simplified(), edtNum->text().simplified(), id, starContact->isChecked()); // change contact data to the database
            }
            wdg->close();
        }
        else
        {
            QPalette palette = edtName->palette();      // ...else set warning placeholder
            palette.setColor(QPalette::PlaceholderText, Qt::red);
            edtName->setPalette(palette);
            edtName->setPlaceholderText("Name must be at least 1 character");
            edtNum->setPalette(palette);
            edtNum->setPlaceholderText("Phone number must be at least 1 character");
        }
    });

    connect(defineContactBox, &QDialogButtonBox::rejected, wdg, &QWidget::close);

    grid->addWidget(edtName);
    grid->addWidget(edtNum);
    grid->addWidget(groupBox);
    grid->addLayout(boxLayout, 3, 0, Qt::AlignCenter);
    grid->addWidget(addGroup);
    grid->addWidget(starContact);
    grid->addWidget(defineContactBox, 6, 0);

    wdg->setLayout(grid);
    wdg->show();
}

void MainMenu::changeContactData(const QModelIndex& ind)  // get contact data and send it to defining method
{
    auto contact = model->getContactData(ind);
    defineContact(true, std::get<PMTableModel::CONTACT_COLUMN::NAME>(contact),
                  std::get<PMTableModel::CONTACT_COLUMN::PHONE>(contact),
                  std::get<PMTableModel::CONTACT_COLUMN::GR_ID>(contact),
                  std::get<PMTableModel::CONTACT_COLUMN::STAR>(contact), ind);
}

void MainMenu::deleteContacts() // delete selected contacts from database
{
    QModelIndexList indexes = view->selectionModel()->selectedRows();
    model->deleteContacts(indexes);
    lblContactsCount->setText("There are " + QString::number(model->rowCount()) + " contact(s) in the phonebook");
}

void MainMenu::searchContacts() // find contact suiting specific requirements
{
    QWidget *wdg = new QWidget(this);
    wdg->setFixedSize(300, 130);
    wdg->setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    wdg->setWindowTitle(appName);

    QGridLayout *grid = new QGridLayout(this);

    QLabel *lbl = new QLabel("Enter name or phone number to start searching", this);
    grid->addWidget(lbl);

    PMLineEdit *edtName = new PMLineEdit(this);
    PMLineEdit *edtPhone = new PMLineEdit(this);
    edtName->setPlaceholderText("Name");
    edtPhone->setPlaceholderText("Phone number");
    grid->addWidget(edtName);
    grid->addWidget(edtPhone);

    QDialogButtonBox *button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    grid->addWidget(button_box, 3, 0);

    connect(button_box, &QDialogButtonBox::accepted, this, [=]
    {
        if(edtName->text().simplified().isEmpty() && edtPhone->text().simplified().isEmpty())
        {
            QPalette palette = lbl->palette();
            palette.setColor(QPalette::WindowText, Qt::red);
            lbl->setPalette(palette);
        }
        else
        {
            view->showRows(edtName->text().simplified(), edtPhone->text().simplified());
            wdg->close();
        }
    }
    );

    connect(button_box, &QDialogButtonBox::rejected, wdg, &QWidget::close);

    wdg->setLayout(grid);
    wdg->show();
}

void MainMenu::changeGroup()    // ask model to change group name or delete it
{
    QWidget *wdg = new QWidget(this);
    wdg->setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    wdg->setMinimumWidth(300);
    wdg->setWindowTitle(appName);

    QGridLayout *grid = new QGridLayout(this);

    QLabel *lbl = new QLabel("Choose a group to change or delete", this);
    grid->addWidget(lbl);

    QComboBox *groupBox = new QComboBox(this);  // combo box with group list
    grid->addWidget(groupBox);
    groupBox->setInsertPolicy(QComboBox::InsertAfterCurrent);
    QMap<int, QString> map = model->getGroups();
    QMap<int, QString>::iterator it;
    for(it = ++map.begin(); it != map.end(); it++)  // user can't change "no group" parameter...
        groupBox->addItem(it.value(), it.key());    // ...so it hasn't been added to this combo box

    PMLineEdit *edtName = new PMLineEdit(this);
    if(groupBox->count() != 0)
    {
        PMPushButton *deleteGroup = new PMPushButton("&Delete group", this, "Delete selected group");
        grid->addWidget(deleteGroup);
        connect(deleteGroup, &PMPushButton::clicked, this, [=]
        {
            model->deleteGroup(groupBox->currentData().toInt());
            QMessageBox::information(this, appName, "Group has been deleted");
            wdg->close();
        });

        edtName->setPlaceholderText("Type new group name");
        grid->addWidget(edtName);
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    grid->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this, edtName, wdg, groupBox]
    {
        if(!edtName->text().simplified().isEmpty()) // if group new name field is not empty...
            model->changeGroupName(edtName->text(), groupBox->currentData().toInt());   // ...change group name
        wdg->close();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, wdg, &QWidget::close);

    wdg->setLayout(grid);
    wdg->show();
}

void MainMenu::exportContacts() // get export settings and ask model to export contacts data
{
    QWidget *wdg = new QWidget();
    wdg->setFixedSize(500,100);
    wdg->setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    wdg->setWindowTitle(appName);

    QGridLayout *grid = new QGridLayout(this);

    PMLineEdit *filePath = new PMLineEdit(this);
    filePath->setPlaceholderText("Choose folder");
    grid->addWidget(filePath, 0, 0);

    PMPushButton *browse = new PMPushButton("&Browse", this, "Find a folder");
    grid->addWidget(browse, 0, 1);

    connect(browse, &PMPushButton::clicked, this, [filePath, this]  // get path for data export
    {
        QString directory = QFileDialog::getExistingDirectory(this, tr("Find Files"), QDir::currentPath());
        filePath->setText(directory);   // show it to the user
    });

    PMRadioButton *radio = new PMRadioButton("Export by group", this, "Export contacts, belonging to a specific group");
    grid->addWidget(radio);
    QComboBox *groupBox = new QComboBox(this);  // combo box with group list
    QMap<int, QString> map = model->getGroups();
    QMap<int, QString>::iterator it;
    for(it = map.begin(); it != map.end(); it++)
    {
        if(it.key() == 1) groupBox->addItem("Export contacts without group", it.key());  // groupID = 1 is a "no group" parameter
            else groupBox->addItem(it.value(), it.key());
    }
    groupBox->setDisabled(true);
    grid->addWidget(groupBox);

    connect(radio, &PMRadioButton::clicked, this, [=]
    {
        if(radio->isChecked())
            groupBox->setDisabled(false);
        else
            groupBox->setDisabled(true);
    });

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    grid->addWidget(buttonBox, 2, 1);

    connect(buttonBox, &QDialogButtonBox::accepted, this, [=]
    {
        const QFileInfo output_dir(filePath->text());
        if (!output_dir.exists() || (!output_dir.isDir()) || (!output_dir.isWritable()))
        {
            QMessageBox::warning(this, "Error", "Output directory does not exist, is not a directory or is not writeable");
        }
        else if(model->rowCount() == 0)
           QMessageBox::warning(this, "Error", "There are no contacts to export");
       else
       {
           if(groupBox->isEnabled())    // export by group if radio button is ON...
               model->exportContactsPDF(filePath->text(), true, groupBox->currentData().toInt());
           else
               model->exportContactsPDF(filePath->text());  // ...else export all contacts
           QMessageBox::information(this, appName, "Contacts have been exported");
           wdg->close();
       }
    });

    connect(buttonBox, &QDialogButtonBox::rejected, wdg, &QWidget::close);

    grid->setColumnStretch(0, 10);
    wdg->setLayout(grid);
    wdg->show();
}
