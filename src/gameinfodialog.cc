#include "gameinfodialog.h"
#include "ui_gameinfodialog.h"

#include <QMessageBox>
#include <QDebug>

#include "gameinfo.h"


/* Implementation of the game information enumerator callback interface.
 * See tads3/gameinfo.h for details.
 */
class QTadsGameInfoEnum: public CTadsGameInfo_enum
{
  public:
	QString gameName;
	QString byLine;
	QString htmlByLine;
	QString email;
	QString desc;
	QString htmlDesc;
	QString version;
	QString date;
	QString published;
	QString lang;
	QString license;
	QString copyRules;
	QString ifid;

	virtual void
	tads_enum_game_info( const char* name, const char* val )
	{
		const QString& valStr = QString::fromUtf8(val);
		const QString& nameStr = QString::fromUtf8(name).toLower();
		if (nameStr == QString::fromAscii("name")) {
			this->gameName = QString::fromAscii("<b><center><font size=\"+1\">") + Qt::escape(valStr)
							 + QString::fromAscii("</font></center></b>");
		} else if (nameStr == QString::fromAscii("byline")) {
			this->byLine = QString::fromAscii("<i><center>") + Qt::escape(valStr)
						   + QString::fromAscii("</center></i>");
		} else if (nameStr == QString::fromAscii("htmlbyline")) {
			this->htmlByLine = QString::fromAscii("<i><center>") + valStr + QString::fromAscii("</center></i>");
		} else if (nameStr == QString::fromAscii("authoremail")) {
			this->email = valStr;
		} else if (nameStr == QString::fromAscii("desc")) {
			this->desc = Qt::escape(valStr).replace(QString::fromAscii("\\n"), QString::fromAscii("<p>"));
		} else if (nameStr == QString::fromAscii("htmldesc")) {
			this->htmlDesc = valStr;
		} else if (nameStr == QString::fromAscii("version")) {
			this->version = valStr;
		} else if (nameStr == QString::fromAscii("releasedate")) {
			this->date = valStr;
		} else if (nameStr == QString::fromAscii("language")) {
			this->lang = valStr;
		} else if (nameStr == QString::fromAscii("licensetype")) {
			this->license = valStr;
		} else if (nameStr == QString::fromAscii("copyingrules")) {
			this->copyRules = valStr;
		} else if (nameStr == QString::fromAscii("firstpublished")) {
			this->published = valStr;
		} else if (nameStr == QString::fromAscii("ifid")) {
			this->ifid = valStr;
		}
	}
};


static void
insertTableRow( QTableWidget* table, const QString& text1, const QString& text2 )
{
	table->insertRow(table->rowCount());
	QTableWidgetItem* item = new QTableWidgetItem(text1);
	item->setFlags(Qt::ItemIsEnabled);
	table->setItem(table->rowCount() - 1, 0, item);
	item = new QTableWidgetItem(text2);
	item->setFlags(Qt::ItemIsEnabled);
	table->setItem(table->rowCount() - 1, 1, item);
}


GameInfoDialog::GameInfoDialog( const QByteArray& fname, QWidget* parent )
  : QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint), ui(new Ui::GameInfoDialog)
{
    ui->setupUi(this);

	CTadsGameInfo info;
	QTadsGameInfoEnum cb;

	info.read_from_file(fname.constData());
	info.enum_values(&cb);

	// Fill out the description.
	QString tmp(cb.gameName + QString::fromAscii("<p>")
				+ (cb.htmlByLine.isEmpty() ? cb.byLine : cb.htmlByLine) + QString::fromAscii("<p>"));
	tmp += cb.htmlDesc.isEmpty() ? cb.desc : cb.htmlDesc;
	ui->description->setHtml(tmp);

	// Fill out the table.
	ui->table->setColumnCount(2);
	if (not cb.version.isEmpty()) {
		insertTableRow(ui->table, tr("Version"), cb.version);
	}

	if (not cb.date.isEmpty()) {
		insertTableRow(ui->table, tr("Date"), cb.date);
	}

	if (not cb.published.isEmpty()) {
		insertTableRow(ui->table, tr("First Published"), cb.published);
	}

	if (not cb.email.isEmpty()) {
		insertTableRow(ui->table, tr("Author email"), cb.email);
	}

	if (not cb.lang.isEmpty()) {
		insertTableRow(ui->table, tr("Language"), cb.lang);
	}

	if (not cb.license.isEmpty()) {
		insertTableRow(ui->table, tr("License Type"), cb.license);
	}

	if (not cb.copyRules.isEmpty()) {
		insertTableRow(ui->table, tr("Copying Rules"), cb.copyRules);
	}

	if (not cb.ifid.isEmpty()) {
		insertTableRow(ui->table, tr("IFID"), cb.ifid);
	}

	ui->table->resizeColumnsToContents();
	ui->table->resizeRowsToContents();
	ui->table->setShowGrid(false);
}


GameInfoDialog::~GameInfoDialog()
{
    delete ui;
}


bool
GameInfoDialog::gameHasMetaInfo( const QByteArray& fname )
{
	CTadsGameInfo info;
	return info.read_from_file(fname.constData());
}
