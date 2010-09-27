#include "aboutqtadsdialog.h"
#include "ui_aboutqtadsdialog.h"

#include "globals.h"
#include "trd.h"
#include "vmvsn.h"


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

	// Construct a string holding all version info.
	QString str;
	str += tr("QTads version:") + "\t" + QTADS_VERSION + "\n\n"
		   + tr("TADS 2 virtual machine:") + "\t" + TADS_RUNTIME_VERSION + "\n"
		   + tr("TADS 3 virtual machine:") + "\t" + T3VM_VSN_STRING + " (" + T3VM_IDENTIFICATION + ")\n\n"
		   + tr("Qt build version:") + "\t" + qVersion() + "\n"
		   + tr("Qt runtime version:") + "\t" + QT_VERSION_STR;
	ui->versionInfoLabel->setText(str);
}


AboutQtadsDialog::~AboutQtadsDialog()
{
	delete ui;
}
