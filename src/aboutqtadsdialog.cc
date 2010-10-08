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
	str += "<table border=\"0\" width=\"100%\"><tr><td>";
	str += tr("QTads version:") + "</td><td>" + QTADS_VERSION + "<br></td></tr><tr><td>"
		   + tr("TADS 2 virtual machine:") + "</td><td>\t" + TADS_RUNTIME_VERSION + "</td></tr><tr><td>"
		   + tr("TADS 3 virtual machine:") + "</td><td>\t" + T3VM_VSN_STRING + " (" + T3VM_IDENTIFICATION
		   + ")<br></td></tr><tr><td>"
		   + tr("Qt build version:") + "</td><td>" + QT_VERSION_STR + "</td></tr><tr><td>"
		   + tr("Qt runtime version:") + "</td><td>" + qVersion() + "</td></tr></table>";
	ui->versionInfoLabel->setText(str);
}


AboutQtadsDialog::~AboutQtadsDialog()
{
	delete ui;
}
