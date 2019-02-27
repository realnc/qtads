#include "aboutqtadsdialog.h"
#include "ui_aboutqtadsdialog.h"

#include "globals.h"
#include "htmlver.h"
#include "trd.h"
#include "vmvsn.h"

AboutQtadsDialog::AboutQtadsDialog(QWidget* parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
    , ui(new Ui::AboutQtadsDialog)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    // On Mac OS X, the dialog should not have any buttons.
    ui->buttonBox->setStandardButtons(QDialogButtonBox::NoButton);
#else
    // Show a "Close" button everywhere else.
    ui->buttonBox->setStandardButtons(QDialogButtonBox::Close);
#endif

    // Construct a string holding all version info.
    QString str;
    str += QString::fromLatin1("<table border=\"0\" width=\"100%\"><tr><td>");
    str += tr("QTads version:") + QString::fromLatin1("</td><td>")
           + QString::fromLatin1(QTADS_VERSION) + QString::fromLatin1("<br></td></tr><tr><td>")
           + tr("HTML TADS version:") + QString::fromLatin1("</td><td>\t")
           + QString::fromLatin1(HTMLTADS_VERSION) + QString::fromLatin1("</td></tr><tr><td>")
           + tr("TADS 2 virtual machine:") + QString::fromLatin1("</td><td>\t")
           + QString::fromLatin1(TADS_RUNTIME_VERSION) + QString::fromLatin1("</td></tr><tr><td>")
           + tr("TADS 3 virtual machine:") + QString::fromLatin1("</td><td>\t")
           + QString::fromLatin1(T3VM_VSN_STRING) + QString::fromLatin1(" (")
           + QString::fromLatin1(T3VM_IDENTIFICATION)
           + QString::fromLatin1(")<br></td></tr><tr><td>") + tr("Qt build version:")
           + QString::fromLatin1("</td><td>") + QString::fromLatin1(QT_VERSION_STR)
           + QString::fromLatin1("</td></tr><tr><td>") + tr("Qt runtime version:")
           + QString::fromLatin1("</td><td>") + QString::fromLatin1(qVersion())
           + QString::fromLatin1("</td></tr></table>");
    ui->versionInfoLabel->setText(str);
}

AboutQtadsDialog::~AboutQtadsDialog()
{
    delete ui;
}
