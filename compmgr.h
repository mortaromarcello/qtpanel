#ifndef COMPMGR_H
#define COMPMGR_H

#include <QThread>

class CompMgr: public QThread
{
	Q_OBJECT
public:
    CompMgr():m_stop(false) {}
    ~CompMgr() {}
public slots:
    void stop() {m_stop = true;}
private:
	void run();
	bool m_stop;
};

#endif // COMPMGR_H
