#ifndef MYDEBUG_H
#define MYDEBUG_H
#include <QtDebug>

//#define __DEBUG__

#ifdef __DEBUG__
#define MyDBG (qDebug()<<"File:"<<__FILE__<<"Line:"<<__LINE__<<"Function:"<<__PRETTY_FUNCTION__<<"\n\t->")
#endif

#endif //MYDEBUG_H__
