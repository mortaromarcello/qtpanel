#include "dialogappletoptions.h"
#include "panelapplication.h"
#include "ui_dialogappletoptions.h"

#include "applicationsmenuapplet.h"
#include "backlightapplet.h"
#include "batteryapplet.h"
#include "clockapplet.h"
#include "demoapplet.h"
#include "dockapplet.h"
#include "docklauncherapplet.h"
#include "launcherapplet.h"
#include "mediaapplet.h"
#include "memoryapplet.h"
#include "pagerapplet.h"
#include "sessionapplet.h"
#include "spacerapplet.h"
#include "trayapplet.h"
#include "volumeapplet.h"

#include "applets.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QTimer>

DialogAppletOptions::DialogAppletOptions(PanelWindow* panelWindow, QVector<Applet*> applets, QWidget* parent)
	: QDialog(parent), m_panelWindow(panelWindow), m_applets(applets)
{
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	QHBoxLayout* appletsLayout = new QHBoxLayout(this);
	mainLayout->addLayout(appletsLayout);
	m_listWidget = new QListWidget(this);
	for (int i = 0; i < m_applets.size(); i++) {
		if (m_applets.at(i)->getNameApplet() == "LauncherApplet") {
			m_listWidget->addItem(QString("%1 (%2)").arg(m_applets.at(i)->getNameApplet()).arg(((LauncherApplet*)m_applets.at(i))->getCommand()));
		}
		else
			m_listWidget->addItem(m_applets.at(i)->getNameApplet());
	}
	QVBoxLayout* buttonsLayout = new QVBoxLayout(this);
	QPushButton* upButton = new QPushButton(QIcon::fromTheme("up"), tr("&Up"), this);
	connect(upButton, SIGNAL(clicked()), this, SLOT(upButtonClick()));
	buttonsLayout->addWidget(upButton);
	QPushButton* downButton = new QPushButton(QIcon::fromTheme("down"), tr("&Down"), this);
	connect(downButton, SIGNAL(clicked()), this, SLOT(downButtonClick()));
	buttonsLayout->addWidget(downButton);
	QPushButton* addButton = new QPushButton(QIcon::fromTheme("add"), tr("&Add"), this);
	connect(addButton, SIGNAL(clicked()), this, SLOT(addButtonClick()));
	buttonsLayout->addWidget(addButton);
	QPushButton* deleteButton = new QPushButton(QIcon::fromTheme("remove"), tr("&Remove"), this);
	connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteButtonClick()));
	buttonsLayout->addWidget(deleteButton);
	QPushButton* configureButton = new QPushButton(QIcon::fromTheme("preferences-other"), tr("&Configure"), this);
	connect(configureButton, SIGNAL(clicked()), this, SLOT(configureButtonClick()));
	buttonsLayout->addWidget(configureButton);
	appletsLayout->addWidget(m_listWidget);
	appletsLayout->addLayout(buttonsLayout);
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	mainLayout->addWidget(buttonBox);
	setLayout(mainLayout);
}

DialogAppletOptions::~DialogAppletOptions()
{
	m_listWidget->clear();
	delete m_listWidget;
}

void DialogAppletOptions::upButtonClick()
{
	int current = m_listWidget->currentRow();
	if (m_applets.at(current) != m_applets.first()) {
		std::swap(m_applets[current-1], m_applets[current]);
		m_listWidget->clear();
		unsigned int nLaunchers = 0;
		for (int i = 0; i < m_applets.size(); i++) {
			if (m_applets.at(i)->getNameApplet() == "LauncherApplet") {
				((LauncherApplet*)m_applets.at(i))->setInstance(++nLaunchers);
				m_listWidget->addItem(QString("%1 (%2)").arg(m_applets.at(i)->getNameApplet()).arg(((LauncherApplet*)m_applets.at(i))->getCommand()));
			}
			else
				m_listWidget->addItem(m_applets.at(i)->getNameApplet());
		}
		m_listWidget->setCurrentRow(current-1);
	}
}

void DialogAppletOptions::downButtonClick()
{
	int current = m_listWidget->currentRow();
	if (m_applets.at(current) != m_applets.last()) {
		std::swap(m_applets[current], m_applets[current+1]);
		m_listWidget->clear();
		unsigned int nLaunchers = 0;
		for (int i = 0; i < m_applets.size(); i++) {
			if (m_applets.at(i)->getNameApplet() == "LauncherApplet") {
				((LauncherApplet*)m_applets.at(i))->setInstance(++nLaunchers);
				m_listWidget->addItem(QString("%1 (%2)").arg(m_applets.at(i)->getNameApplet()).arg(((LauncherApplet*)m_applets.at(i))->getCommand()));
			}
			else
				m_listWidget->addItem(m_applets.at(i)->getNameApplet());
		}
		m_listWidget->setCurrentRow(current+1);
	}
}

void DialogAppletOptions::addButtonClick()
{
	QDialog dialog;
	Ui::DialogAppletOptions settingUi;
	settingUi.setupUi(&dialog);
	settingUi.applet->addItems(listApplets);
	if(dialog.exec() == QDialog::Accepted) {
		int current = 0;
		if (m_listWidget->count())
			current = m_listWidget->currentRow();
		if (settingUi.applet->currentText() == "ApplicationsMenuApplet")
			m_applets.insert(current, new ApplicationsMenuApplet(m_panelWindow));
		else if (settingUi.applet->currentText() == "BacklightApplet")
			m_applets.insert(current, new BacklightApplet(m_panelWindow));
		else if (settingUi.applet->currentText() == "BatteryApplet")
			m_applets.insert(current, new BatteryApplet(m_panelWindow));
		else if (settingUi.applet->currentText() == "ClockApplet")
			m_applets.insert(current, new ClockApplet(m_panelWindow));
		else if (settingUi.applet->currentText() == "DemoApplet")
			m_applets.insert(current, new DemoApplet(m_panelWindow));
		else if (settingUi.applet->currentText() == "DockApplet")
			m_applets.insert(current, new DockApplet(m_panelWindow));
		else if (settingUi.applet->currentText() == "DockLauncherApplet")
			m_applets.insert(current, new DockLauncherApplet(m_panelWindow));
		else if (settingUi.applet->currentText() == "LauncherApplet")
			m_applets.insert(current, new LauncherApplet(m_panelWindow));
		else if (settingUi.applet->currentText() == "MediaApplet")
			m_applets.insert(current, new MediaApplet(m_panelWindow));
		else if (settingUi.applet->currentText() == "MemoryApplet")
			m_applets.insert(current, new MemoryApplet(m_panelWindow));
			else if (settingUi.applet->currentText() == "PagerApplet")
			m_applets.insert(current, new PagerApplet(m_panelWindow));
		else if (settingUi.applet->currentText() == "SessionApplet")
			m_applets.insert(current, new SessionApplet(m_panelWindow));
		else if (settingUi.applet->currentText() == "SpacerApplet")
			m_applets.insert(current, new SpacerApplet(m_panelWindow));
		else if (settingUi.applet->currentText() == "TrayApplet")
			m_applets.insert(current, new TrayApplet(m_panelWindow));
		else if (settingUi.applet->currentText() == "VolumeApplet")
			m_applets.insert(current, new VolumeApplet(m_panelWindow));
		m_applets.at(current)->hide();
		m_applets.at(current)->showConfigurationDialog();
		m_listWidget->clear();
		unsigned int nLaunchers = 0;
		for (int i = 0; i < m_applets.size(); i++) {
			if (m_applets.at(i)->getNameApplet() == "LauncherApplet") {
				((LauncherApplet*)m_applets.at(i))->setInstance(++nLaunchers);
				m_listWidget->addItem(QString("%1 (%2)").arg(m_applets.at(i)->getNameApplet()).arg(((LauncherApplet*)m_applets.at(i))->getCommand()));
			}
			else
				m_listWidget->addItem(m_applets.at(i)->getNameApplet());
		}
		m_listWidget->setCurrentRow(current);
	}
}

void DialogAppletOptions::deleteButtonClick()
{
	int current = m_listWidget->currentRow();
	m_applets.remove(current);
	m_listWidget->clear();
	unsigned int nLaunchers = 0;
	for (int i = 0; i < m_applets.size(); i++) {
		if (m_applets.at(i)->getNameApplet() == "LauncherApplet") {
			((LauncherApplet*)m_applets.at(i))->setInstance(++nLaunchers);
			m_listWidget->addItem(QString("%1 (%2)").arg(m_applets.at(i)->getNameApplet()).arg(((LauncherApplet*)m_applets.at(i))->getCommand()));
		}
		else
			m_listWidget->addItem(m_applets.at(i)->getNameApplet());
	}
	if ((current-1) >= 0)
		m_listWidget->setCurrentRow(current-1);
	else
		m_listWidget->setCurrentRow(0);
}

void DialogAppletOptions::configureButtonClick()
{
	int current = m_listWidget->currentRow();
	m_applets.at(current)->showConfigurationDialog();
}
