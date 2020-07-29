#ifndef LIBRARYLISTVIEW_H
#define LIBRARYLISTVIEW_H

#include <QListView>

class LibraryListView : public QListView
{
    Q_OBJECT
public:
    explicit LibraryListView(QWidget *parent = nullptr);

signals:

};

#endif // LIBRARYLISTVIEW_H
