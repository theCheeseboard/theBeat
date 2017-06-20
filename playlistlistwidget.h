#ifndef PLAYLISTLISTWIDGET_H
#define PLAYLISTLISTWIDGET_H

#include <QListView>

class PlaylistListWidget : public QListView
{
    Q_OBJECT
public:
    explicit PlaylistListWidget(QWidget *parent = nullptr);

    QSize sizeHint() const;
signals:

public slots:
};

#endif // PLAYLISTLISTWIDGET_H
