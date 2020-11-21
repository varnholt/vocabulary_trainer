#pragma once

#include <QMainWindow>

#include "test.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
      Q_OBJECT

   public:
      MainWindow(QWidget *parent = nullptr);
      ~MainWindow();

   protected:
       bool eventFilter(QObject *obj, QEvent *event) override;

   private:
      void showNext();
      void showMenu();
      void initMenu();
      Ui::MainWindow* ui;
      Test _test;
      void fail();
      void pass();
      void flip();

      bool _flipped = false;
};
