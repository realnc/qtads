#ifndef GAMEINFODIALOG_H
#define GAMEINFODIALOG_H

#include <QDialog>

namespace Ui {
    class GameInfoDialog;
}

class GameInfoDialog : public QDialog
{
public:
	explicit GameInfoDialog( const QByteArray& fname, QWidget* parent = 0 );
    ~GameInfoDialog();

	// Checks whether a game file contains any embedded meta information.
	static bool
	gameHasMetaInfo( const QByteArray& fname );

private:
    Ui::GameInfoDialog *ui;
};


#endif // GAMEINFODIALOG_H
