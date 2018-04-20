#include <iostream>
#include "Ini.h"
using namespace std;

int main(int argc, char** argv)
{
    const char* file = "test.ini";

    Ini ini;
    if (!ini.load(file)) {
        return -1;
    }

    ini.dump(stdout);

    if (4 != argc) {
        return 0;
    }

    if ('a' == argv[1][0]) {
        printf("test get arr section:%s key:%s-------------\n", argv[2], argv[3]);
        vector<string> arr = ini.getArr(argv[2], argv[3]);
        for (size_t i=0; i<arr.size(); i++) {
            printf("%s\n", arr[i].c_str());
        }
    } else {
        printf("test get val section:%s key:%s-------------\n", argv[2], argv[3]);
        string v = ini.getVal(argv[2], argv[3]);
        printf("%s\n", v.c_str());
    }

    return 0;
}
