#ifndef INI_H
#define INI_H

#include <string>
#include <map>
#include <vector>

class Ini
{
    /**
     * This enum stores the status for each parsed line (internal use only).
     */
    typedef enum {
        LINE_UNPROCESSED,
        LINE_ERROR,
        LINE_EMPTY,
        LINE_COMMENT,
        LINE_SECTION,
        LINE_VALUE
    } line_status;

public:
    Ini();
    ~Ini();
    bool load(const char* ininame);
    std::string getVal(const char* section, const char* key, const char* def = "");
    std::vector<std::string> getArr(const char* section, const char* key);
    void dump(FILE *fp);
    const char* lastErrmsg();

private:
    line_status parserLine(const char *input_line, char *section,
                           char *key, char *value);
    int  detectdSection(const char* section);
    int  addValue(const char *section, const char* key, const char* val);
    void clear();
    int  onError(const char *format, ...);

private:
    typedef struct {
        std::vector<std::string> val;
    } Val_t;

    typedef std::map<std::string, Val_t*> KeyVal_t;

    std::map<std::string, KeyVal_t*> m_dic;

    /// error information
    std::string m_errmsg;
};

#endif // INI_H
