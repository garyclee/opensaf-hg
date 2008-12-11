/*
 * smidump.c --
 *
 *      Dump a MIB module conforming to a given format.
 *
 * Copyright (c) 1999 Frank Strauss, Technical University of Braunschweig.
 * Copyright (c) 1999 J. Schoenwaelder, Technical University of Braunschweig.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: smidump.c,v 1.66 2002/06/08 12:58:26 schoenw Exp $
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_WIN_H
#include "win.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "smi.h"
#include "shhopt.h"
#include "smidump.h"

/* Global var, to hold view directory path until SMIDUMP completes its
 * processing of generating stubs.
 */
#if ((NCS_MIBLIB != 0) || (NCS_NETSNMP != 0))
char *esmiDirPath = NULL;
char *esmiCopyRightFile = NULL;
#endif

static void help(void);
static void usage(void);
static void version(void);
static void config(char *filename);
static void level(int lev);
static void silent(void);
static void preload(char *module);
static void unified(void);
static void format(char *form);


static int flags;
static SmidumpDriver *driver;
static SmidumpDriver *firstDriver;
static SmidumpDriver *lastDriver;
static SmidumpDriver *defaultDriver;
static char *output;

#if ((NCS_MIBLIB != 0) || (NCS_NETSNMP != 0))
static char *dirPath;
static SmiModule** esmiDumpModulesBuild(int *modc); 
static char *copyRightFile;
#endif

static int opts;
static optStruct *opt;
static optStruct genericOpt[] = {
    /* short long              type        var/func       special       */
    { 'h', "help",           OPT_FLAG,   help,          OPT_CALLFUNC },
    { 'V', "version",        OPT_FLAG,   version,       OPT_CALLFUNC },
    { 'c', "config",         OPT_STRING, config,        OPT_CALLFUNC },
    { 'l', "level",          OPT_INT,    level,         OPT_CALLFUNC },
    { 's', "silent",         OPT_FLAG,   silent,        OPT_CALLFUNC },
    { 'p', "preload",        OPT_STRING, preload,       OPT_CALLFUNC },
    { 'l', "level",          OPT_INT,    level,         OPT_CALLFUNC },
    { 'u', "unified",        OPT_FLAG,   unified,       OPT_CALLFUNC },
    { 'f', "format",         OPT_STRING, format,        OPT_CALLFUNC },
    { 'o', "output",         OPT_STRING, &output,       0            },

#if ((NCS_MIBLIB != 0) || (NCS_NETSNMP != 0))
    { 'd', "dir",            OPT_STRING, &dirPath,      0            },
    { 'R', "dir",            OPT_STRING, &copyRightFile,0            },
#endif

    { 0, 0, OPT_END, 0, 0 }  /* no more options */
};


void *xmalloc(size_t size)
{
    char *m = malloc(size);
    if (! m) {
        fprintf(stderr, "smidump: malloc failed - running out of memory\n");
        exit(1);
    }
    return m;
}



void *xrealloc(void *ptr, size_t size)
{
    char *m = realloc(ptr, size);
    if (! m) {
        fprintf(stderr, "smidump: realloc failed - running out of memory\n");
        exit(1);
    }
    return m;
}



char *xstrdup(const char *s)
{
    char *m = strdup(s);
    if (! m) {
        fprintf(stderr, "smidump: strdup failed - running out of memory\n");
        exit(1);
    }
    return m;
}



void xfree(void *ptr)
{
    free(ptr);
}



void smidumpRegisterDriver(SmidumpDriver *driver)
{
    int i;
    
    if (!firstDriver) {
        firstDriver = driver;
        lastDriver = driver;
    } else {
        lastDriver->next = driver;
        lastDriver = driver;
    }

    for (i = 0; driver->opt && driver->opt[i].type != OPT_END; i++) {
        opt = xrealloc(opt, (opts+1) * sizeof(optStruct));
        memcpy(&opt[opts], &opt[opts-1], sizeof(optStruct));
        opt[opts-1].shortName = 0;
        opt[opts-1].longName  = xmalloc(strlen(driver->name) +
                                 strlen(driver->opt[i].name) + 2);
        sprintf(opt[opts-1].longName, "%s-%s",
                driver->name, driver->opt[i].name);
        opt[opts-1].type      = driver->opt[i].type;
        opt[opts-1].arg       = driver->opt[i].arg;
        opt[opts-1].flags     = driver->opt[i].flags;
        opts++;
    }
}



static void formats()
{
    SmidumpDriver *driver = firstDriver;
    
    for (driver = firstDriver; driver; driver = driver->next) {
        fprintf(stderr, "  %-14s: %s\n", driver->name,
                 driver->descr ? driver->descr : "...");
    }
}



static void usage()
{
    int i;
    SmidumpDriver *driver;
    char *value = NULL;
    
    fprintf(stderr,
            "Usage: smidump [options] [module or path ...]\n"
            "  -V, --version        show version and license information\n"
            "  -h, --help           show usage information\n"
            "  -s, --silent         do not generate any comments\n"
            "  -c, --config=file    load a specific configuration file\n"
            "  -p, --preload=module preload <module>\n"
            "  -l, --level=level    set maximum level of errors and warnings\n"
            "  -f, --format=format  use <format> when dumping (default %s)\n"
            "  -o, --output=name    use <name> when creating names for output files\n"
            "  -u, --unified        print a single unified output of all modules\n"
            "  -R, --CopyRights     Dumps the copy right information specified in the file\n\n",
            defaultDriver ? defaultDriver->name : "none");

    fprintf(stderr, "Supported formats are:\n");
    formats();

    for (driver = firstDriver; driver; driver = driver->next) {
        if (! driver->opt) continue;
        fprintf(stderr, "\nSpecific option for the \"%s\" format:\n",
                driver->name);
        for (i = 0; driver->opt && driver->opt[i].type != OPT_END; i++) {
            int n;
            switch (driver->opt[i].type) {
            case OPT_END:
            case OPT_FLAG:
                value = NULL;
                break;
            case OPT_STRING:
                value = "string";
                break;
            case OPT_INT:
            case OPT_UINT:
            case OPT_LONG:
            case OPT_ULONG:
                value = "number";
                break;
            }
            fprintf(stderr, "  --%s-%s%s%s%n",
                    driver->name, driver->opt[i].name,
                    value ? "=" : "",
                    value ? value : "",
                    &n);
            fprintf(stderr, "%*s%s\n",
                    30-n, "",
                    driver->opt[i].descr ? driver->opt[i].descr : "...");
        }
    }
}



static void help() { usage(); exit(0); }
static void version() { printf("smidump " SMI_VERSION_STRING "\n"); exit(0); }
static void config(char *filename) { smiReadConfig(filename, "smidump"); }
static void level(int lev) { smiSetErrorLevel(lev); }
static void silent() { flags |= SMIDUMP_FLAG_SILENT; }
static void preload(char *module) { smiLoadModule(module); }
static void unified() { flags |= SMIDUMP_FLAG_UNITE; }

static void format(char *form)
{
    for (driver = firstDriver; driver; driver = driver->next) {
        if (strcasecmp(driver->name, form) == 0) {
            break;
        }
    }
    if (!driver) {
        fprintf(stderr, "smidump: invalid dump format `%s'"
                " - supported formats are:\n", form);
        formats();
        exit(1);
    }
}



int main(int argc, char *argv[])
{
    char *modulename;
    SmiModule *smiModule;
    int smiflags, i;
    SmiModule **modv = NULL;
    int modc = 0;
#if ((NCS_MIBLIB != 0) || (NCS_NETSNMP != 0))
    int dirPathLen = 0;
    int copyRightFileLength = 0;
#endif
    output = NULL;
    firstDriver = lastDriver = defaultDriver = NULL;

    opts = sizeof(genericOpt) / sizeof(optStruct);
    opt = xmalloc(sizeof(genericOpt));
    memcpy(opt, genericOpt, sizeof(genericOpt));

    initCm();
    initCorba();
#if 0
    initFig();
#endif
    initIdentifiers();
    initImports();
    initJax();
    initMetrics();
    initMosy();
    initNetsnmp();
#if ((NCS_NETSNMP != 0) || (NCS_MIBLIB !=0))
    initNcsMapi(); 
#endif
#if (NCS_NETSNMP != 0)
    initNcsSynNetsnmp(); 
    initNcsAsynNetsnmp(); 
    initNcsSynNetsnmpNew(); 
#endif /* NCS_NETSNMP */
#if (NCS_MIBLIB != 0)
    initNcsMibLib(); 
    initNcsPssv(); 
#endif /* NCS_MIBLIB */
    initPerl();
    initPython();
    initSming();                defaultDriver = lastDriver;
    initSmi();
    initSmiv3();
    initSppi();
#if 0
    initSql();
#endif
    initScli();
    initTree();
    initTypes();
    initXml();
    initXsd();
    
    for (i = 1; i < argc; i++)
        if ((strstr(argv[i], "-c") == argv[i]) ||
            (strstr(argv[i], "--config") == argv[i])) break;
    if (i == argc) 
        smiInit("smidump");
    else
        smiInit(NULL);

    flags = 0;
    driver = defaultDriver;

    optParseOptions(&argc, argv, opt, 0);

    if (!driver) {
        fprintf(stderr, "smidump: no dump formats registered\n");
        smiExit();
        exit(1);
    }
    
#if ((NCS_MIBLIB != 0) || (NCS_NETSNMP != 0))
    /* For ESMI dump, smidump tool expects a view directory path from home
     * (HJ_HOME) to be given as an argument with an option -d <view_dir_path>
     */
    if((! strcmp(driver->name, "synncsnetsnmp")) || 
       (! strcmp(driver->name, "asynncsnetsnmp")) ||
       (! strcmp(driver->name, "synncsnetsnmp_new")) ||
       (! strcmp(driver->name, "ncsmapi")) ||
       (! strcmp(driver->name, "ncsmiblib")) ||
       (! strcmp(driver->name, "ncspssv")))
    {
       if (!dirPath)
       {
          fprintf(stderr, "\n\nsmidump: view directory path not exists,\n"
                          "missing -d <view_dir_path>\n\n");
          smiExit();
          exit(1);
       }
       /* Copy the input dirPath to esmiDirPath */ 
       dirPathLen = strlen(dirPath);
       esmiDirPath = xmalloc(dirPathLen + 1);
       strcpy(esmiDirPath, dirPath);
       esmiDirPath[dirPathLen] = '\0';

       if (!copyRightFile)
       {
          fprintf(stderr, "\n\nsmidump:No Copy Rights file input,\n"
                          "Generating the Code without copyright information.\n\n");
       }
       else
       {
           int status; 
           struct stat buf;
           memset(&buf, 0, sizeof(struct stat)); 
           status = stat((const char*)copyRightFile, &buf); 
           if (status == 0)
           {
               if (buf.st_size != 0)
               {
                   copyRightFileLength = strlen(copyRightFile);
                   esmiCopyRightFile = xmalloc(copyRightFileLength + 1);
                   memset(esmiCopyRightFile, 0, copyRightFileLength+1);
                   strcpy(esmiCopyRightFile, copyRightFile);
                   esmiCopyRightFile[copyRightFileLength] = '\0';
               }
               else
               {
                  fprintf(stderr, "\n\nWarning:smidump:CopyRight Information file is of size zero \n"); 
                  fprintf(stderr, "Generating the Code without CopyRight information.\n\n");
               }
           }
           else
           {
               /* do the followig validations on the copy rights file */ 
               /* file presence */ 
              fprintf(stderr, "\nWarning: smidump: Error with  CopyRights file: %s : %s", copyRightFile, strerror(errno));
              fprintf(stderr, "\nGenerating the Code without CopyRight information.\n\n");
           }
       }
           
    }
#endif

    smiflags = smiGetFlags();
    smiflags |= SMI_FLAG_ERRORS;
    smiflags |= driver->smiflags;
    smiSetFlags(smiflags);

    if (flags & SMIDUMP_FLAG_UNITE && driver->ignflags & SMIDUMP_DRIVER_CANT_UNITE) {
        fprintf(stderr, "smidump: %s format does not support united output:"
                " ignoring -u\n", driver->name);
        flags = (flags & ~SMIDUMP_FLAG_UNITE);
    }

    if (output && driver->ignflags & SMIDUMP_DRIVER_CANT_OUTPUT) {
        fprintf(stderr, "smidump: %s format does not support output option:"
                " ignoring -o %s\n", driver->name, output);
        output = NULL;
    }

#if ((NCS_NETSNMP == 0) && (NCS_MIBLIB == 0))
    modv = (SmiModule **) xmalloc((argc) * sizeof(SmiModule *));
    modc = 0;
#endif
    for (i = 1; i < argc; i++) {
#if ((NCS_NETSNMP != 0) || (NCS_MIBLIB != 0))
        if((! strcmp(driver->name, "synncsnetsnmp")) || 
           (! strcmp(driver->name, "asynncsnetsnmp")) ||
           (! strcmp(driver->name, "synncsnetsnmp_new")) ||
           (! strcmp(driver->name, "ncsmapi")) ||
           (! strcmp(driver->name, "ncsmiblib")) ||
           (! strcmp(driver->name, "ncspssv")))
        {
           modulename = esmiLoadModule(argv[i]);

           if((! modulename) || (esmiError))
           {
              esmiError = TRUE;
              fprintf(stderr, "\n\nsmidump: cannot generate stubs\n"
                              "due to the error occurred in %s\n\n", argv[i]);
              break;
           }
        }
        else
#endif
           modulename = smiLoadModule(argv[i]);

        smiModule = modulename ? smiGetModule(modulename) : NULL;
        if (smiModule) {
/* Didn't apply for SMIDUMP extensions for NCS */
#if ((NCS_NETSNMP == 0) && (NCS_MIBLIB == 0))
            if ((smiModule->conformance) && (smiModule->conformance < 3)) {
                if (! (flags & SMIDUMP_FLAG_SILENT)) {
                    fprintf(stderr,
                            "smidump: module `%s' contains errors, "
                            "expect flawed output\n",
                            argv[i]);
                }
            }
            modv[modc++] = smiModule;
#endif
        } else {
            fprintf(stderr, "smidump: cannot locate module `%s'\n",
                    argv[i]);
        }
    }


#if ((NCS_MIBLIB != 0) || (NCS_NETSNMP != 0))
    if(! esmiError)
    {  
        modv = esmiDumpModulesBuild(&modc); 
        if ((modv == NULL) || (modc == 0))
        {
            fprintf(stderr, "\nNo module requires code generation...\n"); 
            esmiError = TRUE; 
        }
        else
#endif

    /* Run the driver to generate the required stubs */
    (driver->func)(modc, modv, flags, output);

#if ((NCS_MIBLIB != 0) || (NCS_NETSNMP != 0))
    }
#endif

#if ((NCS_MIBLIB != 0) || (NCS_NETSNMP != 0))
    if(! esmiError)
    {  
       char tempMibFile[40] = ESMI_TEMP_MIB_FILE;
       ESMI_FILE_DELETE(tempMibFile); 
    }
    else
    { 
       char tempMibFile[40] = ESMI_TEMP_MIB_FILE;
       printf("\n NOTE:\n Please don't do any corrections in file: %s \n", tempMibFile);
       printf(" Please do the corrections in the corresponding original MIB file.\n\n");
    }
#endif

    smiExit();

    if (modv) xfree(modv);
    
    return 0;
}

#if ((NCS_MIBLIB != 0) || (NCS_NETSNMP != 0))
static SmiModule** esmiDumpModulesBuild(int *modc) 
{
        SmiModule **modv = NULL;
        SmiModule *smiModulePtr = NULL; 
        int       i = 0; 

        if (modc == NULL)
            return NULL; 

        smiModulePtr = smiGetFirstModule(); 

        /* make a list of modules to be loaded */ 
        while (smiModulePtr)
        {
            if (smiModulePtr->esmi.dumpCode == 1) 
            {
                /* add to the list modules, for which code has to be dumped */ 
                (*modc)++; 
            }

            /* go to the next module */ 
            smiModulePtr = smiGetNextModule(smiModulePtr); 
        }/* end of while */ 

        /* allocate the memory for so many pointers */ 
        if (*modc != 0)
        {
            modv = (SmiModule **) xmalloc((*modc) * sizeof(SmiModule *));
            if (modv == NULL)
                return NULL; 
        }
        else 
            return NULL; 
       
        /* get the first module */  
        smiModulePtr = smiGetFirstModule(); 

        /* make a list of modules to be loaded */ 
        while (smiModulePtr)
        {
            if (smiModulePtr->esmi.dumpCode == 1)
            {
                /* add to the list modules, for which code has to be dumped */ 
                modv[i++] = smiModulePtr; 
            }

            /* go to the next module */ 
            smiModulePtr = smiGetNextModule(smiModulePtr); 
        }/* end of while */ 

        if (i != *modc)
            return NULL;

        return modv; 
}
#endif


