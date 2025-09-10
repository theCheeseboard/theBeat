create table scans
(
    path TEXT not null
        constraint scans_pk
            primary key
);

create table art
(
    hash      TEXT not null
        constraint art_pk
            primary key,
    image     BLOB not null,
    mime_type TEXT not null
);

create table artist
(
    id         INTEGER
        constraint artist_pk
            primary key autoincrement,
    name       TEXT,
    image_hash TEXT
        constraint artist_image_hash_fk
            references art
            on update RESTRICT on delete RESTRICT
);

create table album
(
    id         INTEGER
        constraint album_pk
            primary key autoincrement,
    name       TEXT,
    image_hash TEXT
        constraint album_image_hash_fk
            references art
            on update RESTRICT on delete RESTRICT
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
    image_hash         TEXT
        constraint tracks_image_hash_fk
            references art
            on update RESTRICT on delete RESTRICT,
    file_modified_date REAL
);

create unique index tracks_url_uindex
    on tracks (url);
