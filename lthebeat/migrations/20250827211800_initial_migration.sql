create table artist
(
    id              INTEGER
        constraint artist_pk
            primary key autoincrement,
    name            TEXT,
    image           BLOB,
    image_mime_type TEXT
);

create table album
(
    id              INTEGER
        constraint album_pk
            primary key autoincrement,
    name            TEXT,
    image           BLOB,
    image_mime_type TEXT
);

create table tracks
(
    id                 INTEGER
        constraint tracks_pk
            primary key autoincrement,
    url                TEXT not null,
    name               TEXT,
    album              integer
        constraint tracks_album_id_fk
            references album
            on update cascade on delete set null,
    artist             integer
        constraint tracks_artist_id_fk
            references artist
            on update cascade on delete set null,
    disc               integer,
    track              integer,
    duration           integer,
    image              BLOB,
    image_mime_type    TEXT,
    file_modified_date REAL
);

create unique index tracks_url_uindex
    on tracks (url);
