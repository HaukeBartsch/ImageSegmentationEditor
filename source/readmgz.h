#ifndef READMGZ_H
#define READMGZ_H

#include <QString>
#include <math.h>
#include "volume.h"
#include "Types.h"


class ReadMGZ
{
public:
    ReadMGZ(QString filename);

    std::vector<ScalarVolume> *getVolume();
    bool save(Volume *vol);

private:
    QString filename;
    void swapBytesWord4(void* dp, ulong nn);
    void swapBytesWord2(void* dp, ulong nn);

    void writeInt3(int value, FILE *fp);
    void writeInt2(int value, FILE *fp);
    void writeInt4(int value, FILE *fp);

};

#endif // READMGZ_H
