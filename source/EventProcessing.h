#ifndef EVENTPROCESSING_H
#define EVENTPROCESSING_H


#include <iostream>
#include <QtGui>
#include <QMouseEvent>
#include <QEvent>
#include "mainwindow.h"

class EventProcessing:public QObject
{
  public:
  EventProcessing( MainWindow *w ):QObject() {
      this->w = w;
  };
  ~EventProcessing() {};

  bool eventFilter(QObject* object,QEvent* event) {
      if (event->type() == QEvent::KeyPress) {
          QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);

          // std::cerr << "You Pressed " << keyEvent->text().toStdString() << "\n";
          return w->myKeyPressEvent(object, keyEvent);
      } else if (event->type() == QEvent::MouseMove) {
          QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
          //fprintf(stderr, "Mouse event received at: %d %d\n", mouseEvent->pos().x(), mouseEvent->pos().y());
          return w->mouseEvent(object, mouseEvent);
          // statusBar()->showMessage(QString("Mouse move (%1,%2)").arg(mouseEvent->pos().x()).arg(mouseEvent->pos().y()));
      } else if (event->type() == QEvent::MouseButtonPress) {
              QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
              //fprintf(stderr, "Mouse event received at: %d %d\n", mouseEvent->pos().x(), mouseEvent->pos().y());
              w->myMousePressEvent(object, mouseEvent);
              return false;
              // statusBar()->showMessage(QString("Mouse move (%1,%2)").arg(mouseEvent->pos().x()).arg(mouseEvent->pos().y()));
      } else if (event->type() == QEvent::MouseButtonRelease) {
              QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
              //fprintf(stderr, "Mouse event received at: %d %d\n", mouseEvent->pos().x(), mouseEvent->pos().y());
              return w->myMouseReleaseEvent(object, mouseEvent);
              // statusBar()->showMessage(QString("Mouse move (%1,%2)").arg(mouseEvent->pos().x()).arg(mouseEvent->pos().y()));
      } else if (event->type() == QEvent::MouseButtonDblClick) {
              QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
              //fprintf(stderr, "Mouse event received at: %d %d\n", mouseEvent->pos().x(), mouseEvent->pos().y());
              w->myMouseButtonDblClick(object, mouseEvent);
              return false;
              // statusBar()->showMessage(QString("Mouse move (%1,%2)").arg(mouseEvent->pos().x()).arg(mouseEvent->pos().y()));
      } else if (event->type() == QEvent::Wheel) {
              QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
              return w->myMouseWheelEvent(object, wheelEvent);
      } else {
          // standard event processing
          return QObject::eventFilter(object, event);
      }
      return false;
  }

private:
  MainWindow *w;
};

#endif // EVENTPROCESSING_H
