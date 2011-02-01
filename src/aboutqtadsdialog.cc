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
	str += QString::fromAscii("<table border=\"0\" width=\"100%\"><tr><td>");
	str += tr("QTads version:") + QString::fromAscii("</td><td>") + QString::fromAscii(QTADS_VERSION)
		   + QString::fromAscii("<br></td></tr><tr><td>")
		   + tr("TADS 2 virtual machine:") + QString::fromAscii("</td><td>\t")
		   + QString::fromAscii(TADS_RUNTIME_VERSION) + QString::fromAscii("</td></tr><tr><td>")
		   + tr("TADS 3 virtual machine:") + QString::fromAscii("</td><td>\t")
		   + QString::fromAscii(T3VM_VSN_STRING) + QString::fromAscii(" (")
		   + QString::fromAscii(T3VM_IDENTIFICATION)
		   + QString::fromAscii(")<br></td></tr><tr><td>")
		   + tr("Qt build version:") + QString::fromAscii("</td><td>") + QString::fromAscii(QT_VERSION_STR)
		   + QString::fromAscii("</td></tr><tr><td>")
		   + tr("Qt runtime version:") + QString::fromAscii("</td><td>") + QString::fromAscii(qVersion())
		   + QString::fromAscii("</td></tr></table>");
	ui->versionInfoLabel->setText(str);
}


AboutQtadsDialog::~AboutQtadsDialog()
{
	delete ui;
}
