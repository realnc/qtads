#ifndef ABOUTQTADSDIALOG_H
#define ABOUTQTADSDIALOG_H

#include <QDialog>

#include "config.h"

namespace Ui {
class AboutQtadsDialog;
}

class AboutQtadsDialog: public QDialog
{
    Q_OBJECT

public:
    explicit AboutQtadsDialog(QWidget* parent = 0);
    ~AboutQtadsDialog() override;

private:
    Ui::AboutQtadsDialog* ui;
};

#endif // ABOUTQTADSDIALOG_H
