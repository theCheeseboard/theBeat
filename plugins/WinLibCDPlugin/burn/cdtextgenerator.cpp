#include "cdtextgenerator.h"

#include <QMap>
#include <QBitArray>
#include <tlogger.h>
#include "cdtextcrcgenerator.h"

struct CDTextGeneratorPrivate {
    QList<CDTextGenerator::Track> tracks;
};

CDTextGenerator::CDTextGenerator(QObject* parent) : QObject(parent) {
    d = new CDTextGeneratorPrivate();
}

CDTextGenerator::~CDTextGenerator() {
    delete d;
}

void CDTextGenerator::addTrack(CDTextGenerator::Track trackInfo) {
    d->tracks.append(trackInfo);
}

#include <QFile>
QByteArray CDTextGenerator::generate() {
    //TODO: Figure out Latin-1 modified encoding

    enum Type : char {
        TitleType = 0x80,
        PerformerType,
        SongwriterType,
        ComposerType,
        ArrangerType,
        FooterType = 0x8F
    };

    struct PackSequence {
        QByteArray bytes;
        QList<int> lengths;
    };

    struct Pack {
        char type; // ID1
        char track; // ID2
        char seq; // ID3
        char charPos; // ID4

        char textData[12];


        QByteArray generate() const {
            QByteArray bytes;
            bytes.append(type);
            bytes.append(track);
            bytes.append(seq);
            bytes.append(charPos);
            bytes.append(textData, 12);

            bytes.append(CDTextCrcGenerator::calculateCrc(bytes));

            QBitArray byteData = QBitArray::fromBits(bytes.data(), bytes.length() * 8);
            QBitArray bitArray(byteData.size() + byteData.size() / 3);
            int offset = 0;
            for (int i = 0; i < byteData.size(); i++) {
                if (i != 0 && i % 6 == 0) offset += 2; //Bit array already defaults to 0 so don't bother clearing out the spacing bits
                bitArray.setBit(i + offset, byteData.testBit(i));
            }

            return QByteArray(bitArray.bits(), bitArray.size() / 8);
        }

        static void appendToPackSequence(PackSequence& packSequence, QString textData) {
            QByteArray textBytes = textData.toUtf8();
            textBytes.append('\0');

            packSequence.bytes.append(textBytes);
            packSequence.lengths.append(textBytes.length());
        }

        static QList<Pack> generatePackSequence(char type, char& seq, QMap<char, char>& typeBlocks, PackSequence& packSequence) {
            QList<Pack> packs;

            for (int currentPack = 0; currentPack < (packSequence.bytes.length() / 12) + 1; currentPack++) {
                int track = -1;
                int currentByte = currentPack * 12;

                QList<int> lengths = packSequence.lengths;
                while (currentByte >= 0) {
                    currentByte -= lengths.takeFirst();
                    track++;
                }

                Pack pack;
                pack.type = type;
                pack.track = track;
                pack.seq = seq++;
                pack.charPos = currentByte + packSequence.lengths.at(track);

                memset(pack.textData, 0, 12);
                memcpy(pack.textData, packSequence.bytes.data() + (currentPack * 12), qMin(12, packSequence.bytes.length() - currentPack * 12));

                pack.track &= 0b01111111;
                pack.charPos &= 0x0F;

                packs.append(pack);
            }

            typeBlocks.insert(type, typeBlocks.value(type, 0) + packs.length());

            packSequence.bytes.clear();
            packSequence.lengths.clear();

            return packs;
        }

        static QList<Pack> generateFooterPacks(char& seq, char tracks, QMap<char, char> typeBlocks) {
            QList<Pack> packs;

            packs.append({
                FooterType,
                0,
                seq++,
                0,
                {
                    0x01, //Character code (ASCII)
                    1, //First track #
                    static_cast<char>(tracks - 1), //Last track #
                    0x00, //Mode 2 & Copy Protection Flags,
                    typeBlocks.value(TitleType),
                    typeBlocks.value(PerformerType),
                    typeBlocks.value(SongwriterType),
                    typeBlocks.value(ComposerType),
                    typeBlocks.value(ArrangerType),
                    0, // # of 0x85 type packs
                    0, // # of 0x86 type packs
                    0  // etc.
                }
            });
            packs.append({
                FooterType,
                1,
                seq++,
                0,
                {
                    0, // # of 0x88 type packs
                    0, // etc.
                    0,
                    0,
                    0,
                    0,
                    0,
                    3, // # of footer type packs
                    seq, // Last sequence # for block 0
                    0, // Last sequence # for block 1
                    0, // etc.
                    0
                }
            });
            packs.append({
                FooterType,
                2,
                seq++,
                0,
                {
                    0, // Last sequence # for block 4
                    0, // etc.
                    0,
                    0,
                    0x09, // Language code for block 0 (English)
                    0, // Language code for block 1
                    0, // etc.
                    0,
                    0,
                    0,
                    0,
                    0
                }
            });

            return packs;
        }
    };

    char seq = 0;
    QList<Pack> packs;
    QMap<char, char> typeBlocks;

    PackSequence packSequence;
    for (int i = 0; i < d->tracks.length(); i++) Pack::appendToPackSequence(packSequence, d->tracks.at(i).title);
    packs.append(Pack::generatePackSequence(TitleType, seq, typeBlocks, packSequence));
    for (int i = 0; i < d->tracks.length(); i++) Pack::appendToPackSequence(packSequence, d->tracks.at(i).performer);
    packs.append(Pack::generatePackSequence(PerformerType, seq, typeBlocks, packSequence));
    for (int i = 0; i < d->tracks.length(); i++) Pack::appendToPackSequence(packSequence, d->tracks.at(i).songwriter);
    packs.append(Pack::generatePackSequence(SongwriterType, seq, typeBlocks, packSequence));
    for (int i = 0; i < d->tracks.length(); i++) Pack::appendToPackSequence(packSequence, d->tracks.at(i).composer);
    packs.append(Pack::generatePackSequence(ComposerType, seq, typeBlocks, packSequence));
    for (int i = 0; i < d->tracks.length(); i++) Pack::appendToPackSequence(packSequence, d->tracks.at(i).arranger);
    packs.append(Pack::generatePackSequence(ArrangerType, seq, typeBlocks, packSequence));

    packs.append(Pack::generateFooterPacks(seq, d->tracks.length(), typeBlocks));

    QByteArray packData;
    int pack = 0;
    while (packData.length() + 18 <= (96 * 150)) {
        packData.append(packs.at(pack % packs.length()).generate());
        pack++;
    }

    return packData;
}
