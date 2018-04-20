#include "Ini.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


////////////////////////////////////////////////////////////////////////////////

/**
 * @brief    Convert a string to lowercase.
 * @param    in   String to convert.
 * @param    out Output buffer.
 * @param    len Size of the out buffer.
 * @return   ptr to the out buffer or NULL if an error occured.
 *
 * This function convert a string into lowercase.
 * At most len - 1 elements of the input string will be converted.
 */
static const char * strlwc(const char * in, char *out, size_t len)
{
    unsigned i ;

    if (in==NULL || out == NULL || len==0) return NULL ;
    i=0 ;
    while (in[i] != '\0' && i < len-1) {
        out[i] = (char)tolower((int)in[i]);
        i++ ;
    }
    out[i] = '\0';
    return out ;
}

/**
 * @brief    Duplicate a string
 * @param    s String to duplicate
 * @return   Pointer to a newly allocated string, to be freed with free()
 *
 * This is a replacement for strdup(). This implementation is provided
 * for systems that do not have it.
 */
static char * xstrdup(const char * s)
{
    char * t ;
    size_t len ;
    if (!s)
        return NULL ;

    len = strlen(s) + 1 ;
    t = (char*) malloc(len) ;
    if (t) {
        memcpy(t, s, len) ;
    }
    return t ;
}

/**
 * @brief    Remove blanks at the beginning and the end of a string.
 * @param    str  String to parse and alter.
 * @return   unsigned New size of the string.
 */
static unsigned int strstrip(char * s)
{
    char *last = NULL ;
    char *dest = s;

    if (s==NULL) return 0;

    last = s + strlen(s);
    while (isspace((unsigned char)*s) && *s) s++;
    while (last > s) {
        if (!isspace((unsigned char)*(last-1)))
            break ;
        last -- ;
    }
    *last = (char)0;

    memmove(dest,s,last - s + 1);
    return (unsigned int)(last - s);
}

////////////////////////////////////////////////////////////////////////////////


Ini::Ini()
{

}

Ini::~Ini()
{
    clear();
}

bool Ini::load(const char *ininame)
{
    clear();

#define ASCIILINESZ 1024

    char line    [ASCIILINESZ+1];
    char section [ASCIILINESZ+1];
    char key     [ASCIILINESZ+1];
    char val     [ASCIILINESZ+1];

    int  last = 0;
    int  len;
    int  lineno = 0;
    int  errs=0;

    FILE* in=fopen(ininame, "r");
    if (NULL == in) {
        onError("cannot open %s, %s\n", ininame, strerror(errno));
        return false;
    }

    memset(line,    0, ASCIILINESZ);
    memset(section, 0, ASCIILINESZ);
    memset(key,     0, ASCIILINESZ);
    memset(val,     0, ASCIILINESZ);

    detectdSection(section); // default section is an empty string
    while (fgets(line+last, ASCIILINESZ-last, in)!=NULL) {
        lineno++;
        len = (int)strlen(line) - 1;

        if (len<=0)
            continue;

        /* Safety check against buffer overflows */
        if (line[len]!='\n' && !feof(in)) {
            onError("input line too long in %s (%d)\n", ininame, lineno);
            clear();
            fclose(in);
            return false;
        }

        /* Get rid of \n and spaces at end of line */
        while ( (len>=0)
                && ((line[len]=='\n')
                    || (isspace((unsigned char)line[len]))) ) {
            line[len] = 0;
            len-- ;
        }

        if (len < 0) { /* Line was entirely \n and/or spaces */
            len = 0;
        }

        /* Detect multi-line */
        if (line[len]=='\\') {
            last = len;  /* Multi-line value */
            continue;
        } else {
            last = 0;
        }

        switch (parserLine(line, section, key, val)) {
            case LINE_EMPTY:
            case LINE_COMMENT:
                break;

            case LINE_SECTION:
                detectdSection(section);
                break;

            case LINE_VALUE:
                if (0 != addValue(section, key, val)) {
                    onError("syntax error in %s (%d):\n-> %s\n",
                            ininame, lineno, line);
                    errs++;
                }
                break;

            case LINE_ERROR:
                onError("syntax error in %s (%d):\n-> %s\n",
                        ininame, lineno, line);
                errs++;
                break;

            default:
                break;
        }

        memset(line, 0, ASCIILINESZ);
        last=0;
    }

    if (errs) {
        clear();
    }

    fclose(in);
    return 0==errs;
}

std::string Ini::getVal(const char *section, const char *key, const char* def)
{
    std::string val = def;
    if (NULL == section || NULL == key) {
        return val;
    }

    auto itrS = m_dic.find(section);
    if (itrS == m_dic.end()) {
        return val;
    }
    KeyVal_t* kv = itrS->second;

    std::string k;
    size_t len = strlen(key);
    if (len>1 && '['==key[len-2] && ']'==key[len-1]) { // end with "[]"
        k = std::string(key, len-2);
    } else {
        k = std::string(key);
    }

    auto itrV = kv->find(k);
    if (itrV == kv->end()) {
        return val;
    }
    Val_t* v = itrV->second;
    val = v->val[0];

    return val;
}

std::vector<std::string> Ini::getArr(const char *section, const char *key)
{
    std::vector<std::string> arr;

    auto itrS = m_dic.find(section);
    if (itrS == m_dic.end()) {
        return arr;
    }
    KeyVal_t* kv = itrS->second;

    std::string k;
    size_t len = strlen(key);
    if (len>1 && '['==key[len-2] && ']'==key[len-1]) { // end with "[]"
        k = std::string(key);
    } else {
        k = std::string(key) + "[]";
    }

    auto itrV = kv->find(k);
    if (itrV == kv->end()) {
        return arr;
    }
    Val_t* v = itrV->second;
    arr = v->val;

    return arr;
}

void Ini::dump(FILE* fp)
{
    for (auto itrS=m_dic.begin(); itrS!=m_dic.end(); ++itrS) {
        KeyVal_t* kv = itrS->second;
        if (kv->empty()) {
            continue;
        }

        fprintf(fp, "[%s]\n", itrS->first.c_str());
        for (auto itrV=kv->begin(); itrV!=kv->end(); ++itrV) {
            Val_t* v = itrV->second;
            for (size_t i=0; i<v->val.size(); i++) {
                fprintf(fp, "%s = %s\n", itrV->first.c_str(), v->val[i].c_str());
            }
        }
        fprintf(fp, "\n");
    }
}

const char *Ini::lastErrmsg()
{
    return m_errmsg.c_str();
}

Ini::line_status Ini::parserLine(
        const char* input_line,
        char* section,
        char* key,
        char* value)
{
    line_status sta;
    char * line = NULL;
    size_t      len;

    line = xstrdup(input_line);
    len = strstrip(line);

    sta = LINE_UNPROCESSED;
    if (len < 1) {
        /* Empty line */
        sta = LINE_EMPTY ;
    } else if (line[0]=='#' || line[0]==';') {
        /* Comment line */
        sta = LINE_COMMENT ;
    } else if (line[0]=='[' && line[len-1]==']') {
        /* Section name */
        sscanf(line, "[%[^]]", section);
        strstrip(section);
        strlwc(section, section, len);
        sta = LINE_SECTION ;
    } else if (sscanf (line, "%[^=] = \"%[^\"]\"", key, value) == 2
               ||  sscanf (line, "%[^=] = '%[^\']'",   key, value) == 2) {
        /* Usual key=value with quotes, with or without comments */
        strstrip(key);
        strlwc(key, key, len);
        /* Don't strip spaces from values surrounded with quotes */
        sta = LINE_VALUE ;
    } else if (sscanf (line, "%[^=] = %[^;#]", key, value) == 2) {
        /* Usual key=value without quotes, with or without comments */
        strstrip(key);
        strlwc(key, key, len);
        strstrip(value);
        /*
         * sscanf cannot handle '' or "" as empty values
         * this is done here
         */
        if (!strcmp(value, "\"\"") || (!strcmp(value, "''"))) {
            value[0] = 0;
        }
        sta = LINE_VALUE ;
    } else if (sscanf(line, "%[^=] = %[;#]", key, value) == 2
               || sscanf(line, "%[^=] %[=]", key, value) == 2) {
        /*
         * Special cases:
         * key=
         * key=;
         * key=#
         */
        strstrip(key);
        strlwc(key, key, len);
        value[0]=0 ;
        sta = LINE_VALUE ;
    } else {
        /* Generate syntax error */
        sta = LINE_ERROR ;
    }

    free(line);
    return sta;
}

int Ini::addValue(const char* section, const char *key, const char *val)
{
    auto itrS = m_dic.find(section);
    if (itrS == m_dic.end()) {
        return -1;
    }
    KeyVal_t* kv = itrS->second;

    bool isArr = false;
    size_t len = strlen(key);
    if (len>1 && '[' == key[len-2] && ']' == key[len-1]) { // end with "[]"
        isArr = true;
    }

    Val_t* v = NULL;
    auto itrKv = kv->find(key);
    if (itrKv != kv->end()) {
        v = itrKv->second;
        if (isArr) {
            v->val.push_back(val);
        } else {
            v->val[0] = val;
        }
    } else {
        v = new Val_t();
        v->val.push_back(val);
        kv->insert(std::make_pair(key, v));
    }

    return 0;
}

int Ini::detectdSection(const char *section)
{
    auto itr = m_dic.find(section);
    if (itr == m_dic.end()) {
        m_dic.insert(std::make_pair(std::string(section), new KeyVal_t()));
    }
    return 0;
}

void Ini::clear()
{
    for (auto itrS=m_dic.begin(); itrS!=m_dic.end(); /* do not inc */) {
        KeyVal_t* kv = itrS->second;
        for (auto itrV=kv->begin(); itrV!=kv->end(); /* do not inc */) {
            Val_t* v = itrV->second;
            delete v;
            itrV = kv->erase(itrV);
        }
        delete kv;
        itrS = m_dic.erase(itrS);
    }
}

int Ini::onError(const char *format, ...)
{
    int ret;
    va_list argptr;
    va_start(argptr, format);
    char buffer[1024];
    ret = vsprintf(buffer, format, argptr);
    va_end(argptr);
    m_errmsg = buffer;

    printf(buffer);

    return ret;
}
