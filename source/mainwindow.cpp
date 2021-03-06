#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "gridwidget.h"
#include "loginwidget.h"
#include "progresswidget.h"
#include "summarywidget.h"
#include "settingsdialog.h"
#include "updatewidget.h"

#include "bbapi.h"
#include "player.h"
#include "settings.h"
#include "util.h"
#include "version.h"

#include <QMessageBox>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QProcess>
#include <QUrl>

#include <QFileDialog>
#include <QFile>
#include <QDir>

#include <QDebug>

MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent)
        , loginWidget(new LoginWidget)
        , gridWidget(new GridWidget)
        , progressWidget(new ProgressWidget)
        , summaryWidget(new SummaryWidget)
        , updateWidget(new UpdateWidget)
        , settingsDialog(new SettingsDialog)
        , ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	ui->stackedWidget->addWidget(loginWidget);
	ui->stackedWidget->addWidget(gridWidget);
	ui->stackedWidget->addWidget(progressWidget);
	ui->stackedWidget->addWidget(summaryWidget);
	ui->stackedWidget->addWidget(updateWidget);

	connect(gridWidget, SIGNAL(canProceed(bool)), this, SLOT(enableNextButton(bool)));
	connect(progressWidget, SIGNAL(finished(bool)), ui->nextButton, SLOT(setEnabled(bool)));

	int width = 640, height = 480;
	QPoint point = Util::screenCenter(width, height);
	setGeometry(point.x(), point.y(), width, height);

	setWindowTitle(QString("NTScout %1").arg(Version::toString()));
}

MainWindow::~MainWindow()
{
	delete loginWidget;
	delete gridWidget;
	delete progressWidget;
	delete summaryWidget;
	delete updateWidget;
	delete settingsDialog;
	delete ui;
}

void MainWindow::proceedToCountryWidget()
{
	loginWidget->disableFields();
	loginWidget->setInformation("");
	ui->nextButton->setDisabled(true);

	Countries countries;
	readDataFile(countries);

	QString user = loginWidget->getLogin();
	QString pass = loginWidget->getPassword();
	if (user.isEmpty() || pass.isEmpty()) {
		loginWidget->enableFields();
		loginWidget->setError("Enter your login data first.");
		ui->nextButton->setEnabled(true);
		return;
	}

	BBApi bb(user, pass);
	QString error = bb.login();
	if (!error.isEmpty()) {
		loginWidget->enableFields();
		loginWidget->setError("Unable to login: " + error);
		ui->nextButton->setEnabled(true);
		return;
	}

	enableNextButton(false);

	gridWidget->setCountries(countries);
	ui->stackedWidget->setCurrentWidget(gridWidget);
}

void MainWindow::proceedToProgressWidget()
{
	QMessageBox msgBox;
	msgBox.setTextFormat(Qt::RichText);
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setText(
	        R"(<p align="center" style="font-size:14px">Program is going to start searching now.</p>)");
	msgBox.setInformativeText(
	        R"(<p align="center" style="font-size:14px">Do you want to proceed?</p>)");

	if (msgBox.exec() == QMessageBox::Yes) {
		progressWidget->reset();
		ui->stackedWidget->setCurrentWidget(progressWidget);
		ui->backButton->setEnabled(true);
		ui->nextButton->setDisabled(true);
		QList<SearchValues*> values = gridWidget->getSearchValues();
		progressWidget->start(values);
	}
}

void MainWindow::proceedToSummaryWidget()
{
	ui->backButton->setDisabled(true);
	ui->nextButton->setEnabled(true);

	PlayerList players = progressWidget->getResults();
	summaryWidget->reset();
	summaryWidget->setResults(players);
	ui->stackedWidget->setCurrentWidget(summaryWidget);
}

void MainWindow::goBackToCountryWidget()
{
	ui->backButton->setEnabled(true);
	enableNextButton(false);

	gridWidget->reset();
	ui->stackedWidget->setCurrentWidget(gridWidget);
}

void MainWindow::nextClicked()
{
	switch (ui->stackedWidget->currentIndex()) {
		case Login:
			proceedToCountryWidget();
			break;
		case Country:
			proceedToProgressWidget();
			break;
		case Progress:
			proceedToSummaryWidget();
			break;
		case Summary:
			// prepare for new search
			goBackToCountryWidget();
			break;
	}
}

void MainWindow::backClicked()
{
	switch (ui->stackedWidget->currentIndex()) {
		case Progress:
			progressWidget->stop();
			ui->stackedWidget->setCurrentWidget(gridWidget);
			ui->nextButton->setEnabled(true);
			ui->backButton->setDisabled(true);
			break;
		case Country:
			ui->stackedWidget->setCurrentWidget(summaryWidget);
			ui->nextButton->setEnabled(true);
			ui->backButton->setDisabled(true);
			break;
		case Login:
		case Summary:
			Q_UNREACHABLE();
			break;
	}
}

void MainWindow::updateTriggered()
{
	if (BBApi::name().isEmpty()) {
		QMessageBox::information(this, "Error", "Please login first.", QMessageBox::Ok);
		return;
	}

	ui->stackedWidget->setCurrentWidget(updateWidget);
	updateWidget->onStart();

	//    QString path = qApp->applicationDirPath() + "/Updater";
	//    QStringList params;
	//    params << "-u" << BBApi::getName()
	//           << "-p" << BBApi::getPass()
	//           << "-n" << version;

	//    QProcess::startDetached(path, params);
	//    qApp->exit();
}

void MainWindow::settingsTriggered()
{
	settingsDialog->exec();
}

void MainWindow::reportTriggered()
{
	QDesktopServices::openUrl(QUrl("https://github.com/rszymanski/ntscout/issues"));
}

void MainWindow::aboutTriggered()
{
	QDesktopServices::openUrl(
	        QUrl("http://www.buzzerbeater.com/community/forum/read.aspx?thread=218184&m=1"));
}

void MainWindow::enableNextButton(bool enabled)
{
	ui->nextButton->setEnabled(enabled);
	ui->nextButton->setToolTip(enabled ? ""
	                                   : "You need to select at least "
	                                     "one country and one nationality");
}

void MainWindow::readDataFile(Countries& list)
{
	QString error;
	if (!Util::readCountry(list, error)) {
		QMessageBox msgBox;
		msgBox.setText("Cannot proceed - " + error);
		msgBox.setInformativeText("Would you like to show it's location?");
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msgBox.setDefaultButton(QMessageBox::Yes);

		if (msgBox.exec() == QMessageBox::Yes) {
			QString fileName = QFileDialog::getOpenFileName(
			        this, "Select File", QDir::currentPath(), "Country data (*.dat)");

			if (fileName.isNull() || fileName.isEmpty()) {
				QMessageBox::information(this, "Cannot proceed.", "Quitting.");
				qApp->exit();
			}
			else {
				QDir dir;
				if (!dir.exists("data")) {
					dir.mkdir("data");
				}

				QFile::copy(fileName, "data/country.dat");

				if (!dir.exists("flags")) {
					QMessageBox msgBox;
					msgBox.setText(
					        "It appears there's no flag directory either.");
					msgBox.setInformativeText(
					        "Would you like to show it's location too?");
					msgBox.setStandardButtons(QMessageBox::Yes |
					                          QMessageBox::No);
					msgBox.setDefaultButton(QMessageBox::Yes);

					if (msgBox.exec() == QMessageBox::Yes) {
						QString dir = QFileDialog::getExistingDirectory(
						        this,
						        tr("Select Directory"),
						        QDir::currentPath(),
						        QFileDialog::ShowDirsOnly |
						                QFileDialog::DontResolveSymlinks);

						if (!dir.isNull() && !dir.isEmpty()) {
							Util::copyFolder(dir, "flags");
						}
					}
				}

				readDataFile(list);
			}
		}
		else {
			qApp->exit();
		}
	}
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (ui->stackedWidget->currentIndex() == Progress) {
		QMessageBox msgBox;
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msgBox.setText("Program is still searching for players.");
		msgBox.setInformativeText("Are you sure you want to exit?");

		if (msgBox.exec() == QMessageBox::No) {
			event->ignore();
			return;
		}

		progressWidget->stop();
	}

	Settings::save();
	QMainWindow::closeEvent(event);
}
