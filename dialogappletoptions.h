#ifndef DIALOGAPPLETOPTIONS_H
#define DIALOGAPPLETOPTIONS_H

#include "applet.h"
#include <QDialog>

class QListWidget;
class PanelWindow;

class DialogAppletOptions: public QDialog
{
	Q_OBJECT
public:
	DialogAppletOptions(PanelWindow *panelWindow, QVector<Applet*> applets, QWidget* parent = 0);
	~DialogAppletOptions();
	QVector <Applet*> getApplets() {return m_applets;};
public slots:
	void upButtonClick();
	void downButtonClick();
	void addButtonClick();
	void deleteButtonClick();
	void configureButtonClick();
private:
	PanelWindow* m_panelWindow;
	QVector<Applet*> m_applets;
	QListWidget* m_listWidget;
};

#endif // DIALOGAPPLETOPTIONS_H
