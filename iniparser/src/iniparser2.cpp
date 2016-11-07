
/*-------------------------------------------------------------------------*/
/**
   @file    iniparser.c
   @author  N. Devillard
   @brief   Parser for ini files.
*/
/*--------------------------------------------------------------------------*/
/*---------------------------- Includes ------------------------------------*/

extern "C" {
#include <ctype.h>
#include "iniparser.h"
}
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

/*---------------------------- Defines -------------------------------------*/
#define ASCIILINESZ         (1024)
#define INI_INVALID_KEY     ((char*)-1)

#if defined(_MSC_VER) && defined(_DEBUG)
#define isspace special_isspace
static int special_isspace(int ch)
{
    return (ch == '\t' || ch == ' ');
}
#endif

/*---------------------------------------------------------------------------
                        Private to this module
 ---------------------------------------------------------------------------*/
/**
 * This enum stores the status for each parsed line (internal use only).
 */
typedef enum _line_status_ {
    LINE_UNPROCESSED,
    LINE_ERROR,
    LINE_EMPTY,
    LINE_COMMENT,
    LINE_SECTION,
    LINE_VALUE
} line_status ;

/*-------------------------------------------------------------------------*/
/**
  @brief    Convert a string to lowercase.
  @param    s   String to convert.
  @return   ptr to statically allocated string.

  This function returns a pointer to a statically allocated string
  containing a lowercased version of the input string. Do not free
  or modify the returned string! Since the returned string is statically
  allocated, it will be modified at each function call (not re-entrant).
 */
/*--------------------------------------------------------------------------*/
static char * strlwc(const char * s)
{
    if (s == nullptr) return nullptr;

    static std::string l;
    int s_len = (int)strlen(s);
    if ((int)l.size() < s_len) {
        l.resize(s_len + 1, '\0');
    }

    int i;
    memset(&l[0], 0, s_len + 1);
    i = 0;
    while (i < s_len && s[i]) {
        l[i] = (char)tolower((int)s[i]);
        i++;
    }
    l[s_len] = (char)0;
    return &l[0];
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Remove blanks at the beginning and the end of a string.
  @param    s   String to parse.
  @return   ptr to statically allocated string.

  This function returns a pointer to a statically allocated string,
  which is identical to the input string, except that all blank
  characters at the end and the beg. of the string have been removed.
  Do not free or modify the returned string! Since the returned string
  is statically allocated, it will be modified at each function call
  (not re-entrant).
 */
/*--------------------------------------------------------------------------*/
// trim from start
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
        std::not1(std::ptr_fun<int, int>(isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
        std::not1(std::ptr_fun<int, int>(isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &strstrip(std::string &s) {
    return ltrim(rtrim(s));
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get number of sections in a dictionary
  @param    d   Dictionary to examine
  @return   int Number of sections found in dictionary

  This function returns the number of sections found in a dictionary.
  The test to recognize sections is done on the string stored in the
  dictionary: a section name is given as "section" whereas a key is
  stored as "section:key", thus the test looks for entries that do not
  contain a colon.

  This clearly fails in the case a section name contains a colon, but
  this should simply be avoided.

  This function returns -1 in case of error.
 */
/*--------------------------------------------------------------------------*/
int iniparser_getnsec(dictionary * d)
{
    int i ;
    int nsec ;

    if (d==NULL) return -1 ;
    nsec=0 ;
    for (i=0 ; i<d->size ; i++) {
        if (d->key[i]==NULL)
            continue ;
        if (strchr(d->key[i], ':')==NULL) {
            nsec ++ ;
        }
    }
    return nsec ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get name for section n in a dictionary.
  @param    d   Dictionary to examine
  @param    n   Section number (from 0 to nsec-1).
  @return   Pointer to char string

  This function locates the n-th section in a dictionary and returns
  its name as a pointer to a string statically allocated inside the
  dictionary. Do not free or modify the returned string!

  This function returns NULL in case of error.
 */
/*--------------------------------------------------------------------------*/
char * iniparser_getsecname(dictionary * d, int n)
{
    int i ;
    int foundsec ;

    if (d==NULL || n<0) return NULL ;
    foundsec=0 ;
    for (i=0 ; i<d->size ; i++) {
        if (d->key[i]==NULL)
            continue ;
        if (strchr(d->key[i], ':')==NULL) {
            foundsec++ ;
            if (foundsec>n)
                break ;
        }
    }
    if (foundsec<=n) {
        return NULL ;
    }
    return d->key[i] ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Dump a dictionary to an opened file pointer.
  @param    d   Dictionary to dump.
  @param    f   Opened file pointer to dump to.
  @return   void

  This function prints out the contents of a dictionary, one element by
  line, onto the provided file pointer. It is OK to specify @c stderr
  or @c stdout as output files. This function is meant for debugging
  purposes mostly.
 */
/*--------------------------------------------------------------------------*/
void iniparser_dump(dictionary * d, FILE * f)
{
    int     i ;

    if (d==NULL || f==NULL) return ;
    for (i=0 ; i<d->size ; i++) {
        if (d->key[i]==NULL)
            continue ;
        if (d->val[i]!=NULL) {
            fprintf(f, "[%s]=[%s]\n", d->key[i], d->val[i]);
        } else {
            fprintf(f, "[%s]=UNDEF\n", d->key[i]);
        }
    }
    return ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Save a dictionary to a loadable ini file
  @param    d   Dictionary to dump
  @param    f   Opened file pointer to dump to
  @return   void

  This function dumps a given dictionary into a loadable ini file.
  It is Ok to specify @c stderr or @c stdout as output files.
 */
/*--------------------------------------------------------------------------*/
void iniparser_dump_ini(dictionary * d, FILE * f)
{
    int     i ;
    int     nsec ;

    if (d==NULL || f==NULL) return ;

    nsec = iniparser_getnsec(d);
    if (nsec<1) {
        /* No section in file: dump all keys as they are */
        for (i=0 ; i<d->size ; i++) {
            if (d->key[i]==NULL)
                continue ;
            fprintf(f, "%s = %s\n", d->key[i], d->val[i]);
        }
        return ;
    }
    for (i=0 ; i<nsec ; i++) {
        char *secname = iniparser_getsecname(d, i) ;
        iniparser_dumpsection_ini(d, secname, f) ;
    }
    fprintf(f, "\n");
    return ;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Save a dictionary section to a loadable ini file
  @param    d   Dictionary to dump
  @param    s   Section name of dictionary to dump
  @param    f   Opened file pointer to dump to
  @return   void

  This function dumps a given section of a given dictionary into a loadable ini
  file.  It is Ok to specify @c stderr or @c stdout as output files.
 */
/*--------------------------------------------------------------------------*/
void iniparser_dumpsection_ini(dictionary * d, char * s, FILE * f)
{
    int     j;
    char    keym[ASCIILINESZ + 1];
    int     seclen;

    if (d == NULL || f == NULL) return;
    if (!iniparser_find_entry(d, s)) return;

    seclen = (int)strlen(s);
    fprintf(f, "\n[%s]\n", s);
    sprintf(keym, "%s:", s);
    for (j = 0; j < d->size; j++) {
        if (d->key[j] == NULL)
            continue;
        if (!strncmp(d->key[j], keym, seclen + 1)) {
            fprintf(f,
                "%-30s = %s\n",
                d->key[j] + seclen + 1,
                d->val[j] ? d->val[j] : "");
        }
    }
    fprintf(f, "\n");
    return;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get the number of keys in a section of a dictionary.
  @param    d   Dictionary to examine
  @param    s   Section name of dictionary to examine
  @return   Number of keys in section
 */
/*--------------------------------------------------------------------------*/
int iniparser_getsecnkeys(dictionary * d, char * s)
{
    int     seclen, nkeys;
    char    keym[ASCIILINESZ + 1];
    int j;

    nkeys = 0;

    if (d == NULL) return nkeys;
    if (!iniparser_find_entry(d, s)) return nkeys;

    seclen = (int)strlen(s);
    sprintf(keym, "%s:", s);

    for (j = 0; j < d->size; j++) {
        if (d->key[j] == NULL)
            continue;
        if (!strncmp(d->key[j], keym, seclen + 1))
            nkeys++;
    }

    return nkeys;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get the number of keys in a section of a dictionary.
  @param    d   Dictionary to examine
  @param    s   Section name of dictionary to examine
  @return   pointer to statically allocated character strings

  This function queries a dictionary and finds all keys in a given section.
  Each pointer in the returned char pointer-to-pointer is pointing to
  a string allocated in the dictionary; do not free or modify them.
  
  This function returns NULL in case of error.
 */
/*--------------------------------------------------------------------------*/
char ** iniparser_getseckeys(dictionary * d, char * s)
{
    char **keys;

    int i, j;
    char    keym[ASCIILINESZ + 1];
    int     seclen, nkeys;

    keys = NULL;

    if (d == NULL) return keys;
    if (!iniparser_find_entry(d, s)) return keys;

    nkeys = iniparser_getsecnkeys(d, s);

    keys = (char**)malloc(nkeys * sizeof(char*));
    if (NULL == keys) {
        return keys;
    }

    seclen = (int)strlen(s);
    sprintf(keym, "%s:", s);

    i = 0;

    for (j = 0; j < d->size; j++) {
        if (d->key[j] == NULL)
            continue;
        if (!strncmp(d->key[j], keym, seclen + 1)) {
            keys[i] = d->key[j];
            i++;
        }
    }

    return keys;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key
  @param    d       Dictionary to search
  @param    key     Key string to look for
  @param    def     Default value to return if key not found.
  @return   pointer to statically allocated character string

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  the pointer passed as 'def' is returned.
  The returned char pointer is pointing to a string allocated in
  the dictionary, do not free or modify it.
 */
/*--------------------------------------------------------------------------*/
char * iniparser_getstring(dictionary * d, const char * key, char * def)
{
    char * lc_key;
    char * sval;

    if (d == NULL || key == NULL)
        return def;

    lc_key = strlwc(key);
    sval = dictionary_get(d, lc_key, def);
    return sval;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key, convert to an int
  @param    d Dictionary to search
  @param    key Key string to look for
  @param    notfound Value to return in case of error
  @return   integer

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  the notfound value is returned.

  Supported values for integers include the usual C notation
  so decimal, octal (starting with 0) and hexadecimal (starting with 0x)
  are supported. Examples:

  "42"      ->  42
  "042"     ->  34 (octal -> decimal)
  "0x42"    ->  66 (hexa  -> decimal)

  Warning: the conversion may overflow in various ways. Conversion is
  totally outsourced to strtol(), see the associated man page for overflow
  handling.

  Credits: Thanks to A. Becker for suggesting strtol()
 */
/*--------------------------------------------------------------------------*/
int iniparser_getint(dictionary * d, const char * key, int notfound)
{
    char *str;

    str = iniparser_getstring(d, key, INI_INVALID_KEY);
    if (str == INI_INVALID_KEY) return notfound;
    return (int)strtol(str, NULL, 0);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key, convert to a double
  @param    d Dictionary to search
  @param    key Key string to look for
  @param    notfound Value to return in case of error
  @return   double

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  the notfound value is returned.
 */
/*--------------------------------------------------------------------------*/
double iniparser_getdouble(dictionary * d, const char * key, double notfound)
{
    char *str;

    str = iniparser_getstring(d, key, INI_INVALID_KEY);
    if (str == INI_INVALID_KEY) return notfound;
    return atof(str);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Get the string associated to a key, convert to a boolean
  @param    d Dictionary to search
  @param    key Key string to look for
  @param    notfound Value to return in case of error
  @return   integer

  This function queries a dictionary for a key. A key as read from an
  ini file is given as "section:key". If the key cannot be found,
  the notfound value is returned.

  A true boolean is found if one of the following is matched:

  - A string starting with 'y'
  - A string starting with 'Y'
  - A string starting with 't'
  - A string starting with 'T'
  - A string starting with '1'

  A false boolean is found if one of the following is matched:

  - A string starting with 'n'
  - A string starting with 'N'
  - A string starting with 'f'
  - A string starting with 'F'
  - A string starting with '0'

  The notfound value returned if no boolean is identified, does not
  necessarily have to be 0 or 1.
 */
/*--------------------------------------------------------------------------*/
int iniparser_getboolean(dictionary * d, const char * key, int notfound)
{
    char    *c;
    int     ret;

    c = iniparser_getstring(d, key, INI_INVALID_KEY);
    if (c == INI_INVALID_KEY) return notfound;
    if (c[0] == 'y' || c[0] == 'Y' || c[0] == '1' || c[0] == 't' || c[0] == 'T') {
        ret = 1;
    }
    else if (c[0] == 'n' || c[0] == 'N' || c[0] == '0' || c[0] == 'f' || c[0] == 'F') {
        ret = 0;
    }
    else {
        ret = notfound;
    }
    return ret;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Finds out if a given entry exists in a dictionary
  @param    ini     Dictionary to search
  @param    entry   Name of the entry to look for
  @return   integer 1 if entry exists, 0 otherwise

  Finds out if a given entry exists in the dictionary. Since sections
  are stored as keys with NULL associated values, this is the only way
  of querying for the presence of sections in a dictionary.
 */
/*--------------------------------------------------------------------------*/
int iniparser_find_entry(
    dictionary  *   ini,
    const char  *   entry
)
{
    int found = 0;
    if (iniparser_getstring(ini, entry, INI_INVALID_KEY) != INI_INVALID_KEY) {
        found = 1;
    }
    return found;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Set an entry in a dictionary.
  @param    ini     Dictionary to modify.
  @param    entry   Entry to modify (entry name)
  @param    val     New value to associate to the entry.
  @return   int 0 if Ok, -1 otherwise.

  If the given entry can be found in the dictionary, it is modified to
  contain the provided value. If it cannot be found, -1 is returned.
  It is Ok to set val to NULL.
 */
/*--------------------------------------------------------------------------*/
int iniparser_set(dictionary * ini, const char * entry, const char * val)
{
    return dictionary_set(ini, strlwc(entry), val);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Delete an entry in a dictionary
  @param    ini     Dictionary to modify
  @param    entry   Entry to delete (entry name)
  @return   void

  If the given entry can be found, it is deleted from the dictionary.
 */
/*--------------------------------------------------------------------------*/
void iniparser_unset(dictionary * ini, const char * entry)
{
    dictionary_unset(ini, strlwc(entry));
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Load a single line from an INI file
  @param    input_line  Input line, may be concatenated multi-line input
  @param    section     Output space to store section
  @param    key         Output space to store key
  @param    value       Output space to store value
  @return   line_status value
 */
/*--------------------------------------------------------------------------*/
static line_status iniparser_line(
    const std::string& input_line,
    std::string& section,
    std::string& key,
    std::string& value)
{
    line_status sta;
    std::string line(input_line);
    int         len;

    strstrip(line);
    len = (int)line.length();

    key.resize(ASCIILINESZ + 1);
    value.resize(len);

    sta = LINE_UNPROCESSED;
    if (len < 1) {
        /* Empty line */
        sta = LINE_EMPTY;
    }
    else if (line[0] == '#' || line[0] == ';') {
        /* Comment line */
        sta = LINE_COMMENT;
    }
    else if (line[0] == '[' && line[len - 1] == ']') {
        /* Section name */
        char sec[ASCIILINESZ + 1];
        sscanf(line.c_str(), "[%[^]]", sec);
        section = sec;
        strstrip(section);
        section = strlwc(section.c_str());
        sta = LINE_SECTION;
    }
    else if (sscanf(line.c_str(), "%[^=] = \"%[^\"]\"", &key[0], &value[0]) == 2
        || sscanf(line.c_str(), "%[^=] = '%[^\']'", &key[0], &value[0]) == 2
        || sscanf(line.c_str(), "%[^=] = %[^;#]", &key[0], &value[0]) == 2) {
        /* Usual key=value, with or without comments */
        key.resize(strlen(key.c_str()));
        value.resize(strlen(value.c_str()));

        strstrip(key);
        key = strlwc(key.c_str());
        strstrip(value);
        /*
         * sscanf cannot handle '' or "" as empty values
         * this is done here
         */
        if (!strcmp(value.c_str(), "\"\"") || (!strcmp(value.c_str(), "''"))) {
            value.clear();
        }
        sta = LINE_VALUE;
    }
    else if (sscanf(line.c_str(), "%[^=] = %[;#]", &key[0], &value[0]) == 2
        || sscanf(line.c_str(), "%[^=] %[=]", &key[0], &value[0]) == 2) {
        /*
         * Special cases:
         * key=
         * key=;
         * key=#
         */
        key.resize(strlen(key.c_str()));
        value.resize(strlen(value.c_str()));

        strstrip(key);
        key = strlwc(key.c_str());
        value.clear();
        sta = LINE_VALUE;
    }
    else {
        /* Generate syntax error */
        sta = LINE_ERROR;
    }
    return sta;
}

static bool get_line(std::ifstream& istream, std::string &line)
{
    line.clear();
    do {
        if (std::getline(istream, line)) {
            return true;
        }
        else {
            return false;
        }
    } while (true);

    return false;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Parse an ini file and return an allocated dictionary object
  @param    ininame Name of the ini file to read.
  @return   Pointer to newly allocated dictionary

  This is the parser for ini files. This function is called, providing
  the name of the file to be read. It returns a dictionary object that
  should not be accessed directly, but through accessor functions
  instead.

  The returned dictionary must be freed using iniparser_freedict().
 */
/*--------------------------------------------------------------------------*/
dictionary * iniparser_load(const char * ininame)
{
    std::string line, section, key, tmp, val;
    int  lineno = 0;
    int  errs = 0;

    dictionary * dict;

    std::ifstream in(ininame);
    if (!in.good()) {
        fprintf(stderr, "iniparser: cannot open %s\n", ininame);
        return NULL;
    }

    {
        // skip BOM for UTF8
        int b1 = in.get();
        int b2 = in.get();
        int b3 = in.get();
        if (b1 == 0XEF && b2 == 0XBB && b3 == 0XBF) {
            // found BOM for UTF8, ignore
        }
        else {
            in.close();
            in = std::ifstream(ininame);
        }
    }

    dict = dictionary_new(0);
    if (!dict) {
        in.close();
        return nullptr;
    }

    std::string line_tmp;
    while (get_line(in, line_tmp) == true) {
        line += line_tmp;
        int  len;

        lineno++;
        len = (int)line.length() - 1;
        if (len <= 0)
            continue;

        /* Get rid of \n and spaces at end of line */
        while ((len >= 0) &&
            ((line[len] == '\n') || (isspace(line[len])))) {
            line[len] = 0;
            len--;
        }
        /* Detect multi-line */
        if (line[len] == '\\' && len > 1 && isspace(line[len - 1])) {
            /* Multi-line value */
            line.pop_back();
            continue;
        }

        switch (iniparser_line(line, section, key, val)) {
        case LINE_EMPTY:
        case LINE_COMMENT:
            break;

        case LINE_SECTION:
            errs = dictionary_set(dict, section.c_str(), nullptr);
            break;

        case LINE_VALUE:
            tmp = section;
            tmp += ':';
            tmp += key;
            errs = dictionary_set(dict, tmp.c_str(), val.c_str());
            break;

        case LINE_ERROR:
            fprintf(stderr, "iniparser: syntax error in %s (%d):\n",
                ininame, lineno);
            fprintf(stderr, "-> %s\n", line.c_str());
            errs++;
            break;

        default:
            break;
        }

        line.clear();
        if (errs < 0) {
            fprintf(stderr, "iniparser: memory allocation failure\n");
            break;
        }
    }
    if (errs) {
        dictionary_del(dict);
        dict = nullptr;
    }
    in.close();
    return dict;
}

/*-------------------------------------------------------------------------*/
/**
  @brief    Free all memory associated to an ini dictionary
  @param    d Dictionary to free
  @return   void

  Free all memory associated to an ini dictionary.
  It is mandatory to call this function before the dictionary object
  gets out of the current context.
 */
/*--------------------------------------------------------------------------*/
void iniparser_freedict(dictionary * d)
{
    dictionary_del(d);
}

/* vim: set ts=4 et sw=4 tw=75 */
