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

       void mousePressEvent(QMouseEvent* event) override;
       void mouseDoubleClickEvent(QMouseEvent* event) override;
       void mouseMoveEvent(QMouseEvent* event) override;

   private slots:
       void toggleMaximize();

   private:
      void showNext();
      void showMenu();
      void initMenu();
      void initWordStyle();
      void updateWordStyle(int32_t score);

      void fail();
      void pass();
      void flip();

      Ui::MainWindow* ui = nullptr;
      Test _test;
      bool _flipped = false;
      std::array<int32_t, 2> _mouse_pos;

      QString _word_stylesheet;
      QColor _word_color_default;
      QColor _word_color_hover;
};
