#ifndef ABOUTQTADSDIALOG_H
#define ABOUTQTADSDIALOG_H

#include <QDialog>

namespace Ui {
    class AboutQtadsDialog;
}

class AboutQtadsDialog : public QDialog
{
public:
    explicit AboutQtadsDialog(QWidget *parent = 0);
    ~AboutQtadsDialog();

private:
    Ui::AboutQtadsDialog *ui;
};

#endif // ABOUTQTADSDIALOG_H
