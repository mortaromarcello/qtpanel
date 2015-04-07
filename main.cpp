#include "config.h"
#include "panelapplication.h"
#include <QTranslator>
#include <signal.h>
#include <stdio.h>
#include <QLocale>
#include <QX11Info>

void signalHandler(int signal);

int main(int argc, char** argv)
{
	int ret;
	// configure app's reaction to OS signals
	struct sigaction act, oact;
	memset((void*)&act, 0, sizeof(struct sigaction));
	memset((void*)&oact, 0, sizeof(struct sigaction));
	act.sa_flags = 0;
	act.sa_handler = &signalHandler;
	sigaction(SIGINT, &act, &oact);
	sigaction(SIGKILL, &act, &oact);
	sigaction(SIGQUIT, &act, &oact);
	sigaction(SIGSTOP, &act, &oact);
	sigaction(SIGTERM, &act, &oact);
	sigaction(SIGSEGV, &act, &oact);
	PanelApplication app(argc, argv);
	QTranslator translator;
	translator.load(QLocale::system(), app.getApplicationName(), "_", qtpanel_TRANSLATIONS_TARGET);
	app.installTranslator(&translator);
	app.init();
	ret = app.exec();
	return ret;
}

void signalHandler(int signal)
{
	//print received signal
	switch(signal){
		case SIGINT:
		case SIGKILL:
		case SIGQUIT:
		case SIGSTOP:
		case SIGTERM:
		case SIGSEGV:
			break;
		default:
			break;
	}
	
	//app ends as expected
	fprintf(stderr, "Thus ends!!\n");
	QApplication::quit();
}
