#include "aboutqtadsdialog.h"
#include "ui_aboutqtadsdialog.h"


AboutQtadsDialog::AboutQtadsDialog(QWidget *parent)
: QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint), ui(new Ui::AboutQtadsDialog)
{
	ui->setupUi(this);

#ifdef Q_WS_MAC
	// On Mac OS X, the dialog should not have any buttons.
	ui->buttonBox->setStandardButtons(QDialogButtonBox::NoButton);
#else
	// Show a "Close" button everywhere else.
	ui->buttonBox->setStandardButtons(QDialogButtonBox::Close);
#endif
}


AboutQtadsDialog::~AboutQtadsDialog()
{
	delete ui;
}
