#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QKeyEvent>


namespace
{

constexpr auto title_label_css = R"(
   QLabel
   {
      color: #cccccc;
      background-color: #333333;
   }
)";

constexpr auto title_buttons_css = R"(
   QPushButton
   {
      color: #ffffff;
      background-color: #333333;
      font-weight: normal;
   }

   QPushButton:hover
   {
      background-color: #444444;
   }
)";

constexpr auto main_css = R"(
   QWidget
   {
      background-color: #333333;
   }

   QListWidget
   {
      color: #888888;
   }

   QCheckBox
   {
      color: #888888;
      background-color: #444444;
      font-weight: bold;
      border: none;
      padding: 5px;
   }

   QCheckBox:active
   {
      color: #1d90cd;
   }

   QCheckBox:hover
   {
      background-color: #505050;
   }

   QLabel
   {
      color: #888888;
      background-color: #444444;
      font-weight: bold;
   }

   QLabel:active
   {
      color: #1d90cd;
   }

   QPushButton
   {
      color: #888888;
      background-color: #444444;
      font-weight: bold;
      border: none;
      padding: 5px;
   }

   QPushButton:active
   {
      color: #ffffff;
   }

   QPushButton:hover
   {
      color: #1d90cd;
      background-color: #505050;
   }
)";

}

MainWindow::MainWindow(QWidget *parent)
   : QMainWindow(parent)
   , ui(new Ui::MainWindow)
{
   ui->setupUi(this);
   ui->statusbar->hide();
   ui->menubar->hide();

   ui->_title->setStyleSheet(title_label_css);
   ui->_close->setStyleSheet(title_buttons_css);
   ui->_maximize->setStyleSheet(title_buttons_css);
   ui->_minimize->setStyleSheet(title_buttons_css);
   setStyleSheet(main_css);

   setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

   connect(
      ui->_close,
      &QPushButton::clicked,
      qApp,
      &QApplication::exit
   );

   connect(
      ui->_maximize,
      &QPushButton::clicked,
      this,
      [this]()
      {
         if (isMaximized())
         {
            showNormal();
            ui->_maximize->setText("☐");
         }
         else
         {
            showMaximized();
            ui->_maximize->setText("🗗");
         }
      }
   );

   connect(
      ui->_minimize,
      &QPushButton::clicked,
      this,
      &MainWindow::showMinimized
   );


   setFocusPolicy(Qt::StrongFocus);

   initMenu();
   showMenu();

   qApp->installEventFilter(this);

   connect(
      ui->_start,
      &QPushButton::clicked,
      this,
      [this](){
         if (!ui->_files->currentItem())
         {
            return;
         }
         _test._flipped = ui->_flip->isChecked();
         _test.load(ui->_files->currentItem()->text().toStdString());
         showNext();
         ui->_stack->setCurrentIndex(0);
      }
   );

   connect(
      ui->_pass,
      &QPushButton::clicked,
      this,
      [this](){
         pass();
      }
   );

   connect(
      ui->_fail,
      &QPushButton::clicked,
      this,
      [this](){
         fail();
      }
   );

   connect(
      ui->_word,
      &QPushButton::clicked,
      this,
      [this](){
         flip();
      }
   );
}


MainWindow::~MainWindow()
{
   delete ui;
}


bool MainWindow::eventFilter(QObject* /*obj*/, QEvent *event)
{
   if (ui->_stack->currentIndex() == 1)
   {
      return false;
   }

   if (event->type() == QEvent::KeyPress)
   {
      QKeyEvent* keyEvent = dynamic_cast<QKeyEvent *>(event);

      if (keyEvent->key() == Qt::Key_Left)
      {
         fail();
         return true;
      }
      else if (keyEvent->key() == Qt::Key_Right)
      {
         pass();
         return true;
      }
      else if (keyEvent->key() == Qt::Key_Space)
      {
         flip();
         return true;
      }
      else if (keyEvent->key() == Qt::Key_Escape)
      {
         showMenu();
         return true;
      }
   }
   return false;
}


void MainWindow::showNext()
{
   _flipped = false;
   auto nextWord = _test._words[_test._current]._language_1;
   ui->_word->setText(QString::fromStdString(nextWord).trimmed());
}


void MainWindow::showMenu()
{
   ui->_stack->setCurrentIndex(1);
}


void MainWindow::initMenu()
{
   const auto files = QDir::current().entryList({"*.csv"}, QDir::Filter::Files);
   ui->_files->insertItems(0, files);
   if (!files.empty())
   {
      ui->_files->setCurrentRow(0);
   }
}


void MainWindow::fail()
{
   _test.fail();
   if (_test.next())
   {
      showNext();
   }
   else
   {
      showMenu();
   }
}


void MainWindow::pass()
{
   _test.pass();
   if (_test.next())
   {
      showNext();
   }
   else
   {
      showMenu();
   }
}


void MainWindow::flip()
{
   _flipped = !_flipped;

   auto translation =
         _flipped
            ? _test._words[_test._current]._language_2
            : _test._words[_test._current]._language_1;

   ui->_word->setText(QString::fromStdString(translation).trimmed());
}


void MainWindow::mousePressEvent(QMouseEvent* event)
{
  _mouse_pos[0] = event->position().x();
  _mouse_pos[1] = event->position().y();
}


void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
   move(
      event->globalPosition().x() - _mouse_pos[0],
      event->globalPosition().y() - _mouse_pos[1]
   );
}

