#ifndef GAMEINFODIALOG_H
#define GAMEINFODIALOG_H

#include <QDialog>
#include "gameinfo.h"

/* Implementation of the game information enumerator callback interface.
 * See tads3/gameinfo.h for details.
 */
class QTadsGameInfoEnum: public CTadsGameInfo_enum {
  public:
	QString gameName;
	QString plainGameName; // Game name but without any HTML markup.
	QString headline;
	QString byLine;
	QString htmlByLine;
	QString email;
	QString desc;
	QString htmlDesc;
	QString version;
	QString published;
	QString date;
	QString lang;
	QString series;
	QString seriesNumber;
	QString genre;
	QString forgiveness;
	QString license;
	QString copyRules;
	QString ifid;

	virtual void
	tads_enum_game_info( const char* name, const char* val );
};


namespace Ui {
    class GameInfoDialog;
}

class GameInfoDialog: public QDialog {
	Q_OBJECT

  public:
	explicit GameInfoDialog( const QByteArray& fname, QWidget* parent = 0 );
    ~GameInfoDialog();

	// Checks whether a game file contains any embedded meta information.
	static bool
	gameHasMetaInfo( const QByteArray& fname );

	static QTadsGameInfoEnum
	getMetaInfo( const QByteArray& fname );

  private:
    Ui::GameInfoDialog *ui;
};


#endif // GAMEINFODIALOG_H
