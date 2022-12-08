/* MIT License
 *
 * Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*******************************************************************************
    DESCRIPTION
*******************************************************************************/
/**
 * @brief  Main entry point
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include <LittleFS.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Choose filesystem here. */
#define FILESYSTEM  LittleFS

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static void createDirectoryRecursively(fs::FS& fs, const String& path, const String& baseDirName, const String& baseFileName, uint32_t dirDepth, uint32_t fileCnt);
static void listRecursively(File& path, bool show);
static void listRootRecursively(fs::FS& fs, bool show);

/******************************************************************************
 * Variables
 *****************************************************************************/

/** Serial interface baudrate. */
static const uint32_t   SERIAL_BAUDRATE = 115200U;

/** Module tag used for logging system. */
static const char*      TAG             = "main";

/** Number of directories to create on the same directory level. */
static const uint32_t   DIR_CNT         = 5;

/** Directory depth */
static const uint32_t   DIR_DEPTH       = 5;

/** Number of files to create per directory. */
static const uint32_t   FILE_CNT        = 5;

/** Number of string function tests. */
static const uint32_t   STR_FUNC_CNT    = 10000;

/******************************************************************************
 * External functions
 *****************************************************************************/

/**
 * Setup the system.
 */
void setup()
{
    bool    isError = false;

    Serial.begin(SERIAL_BAUDRATE);

    ESP_LOGI(TAG, "Mount filesystem.");

    if (false == FILESYSTEM.begin(true))
    {
        ESP_LOGE(TAG, "Mounting filesystem failed.");
        isError = true;
    }
    else
    {
        ESP_LOGI(TAG, "Format filesystem.");

        if (false == FILESYSTEM.format())
        {
            ESP_LOGE(TAG, "Formatting filesystem failed.");
            isError = true;
        }
    }

    if (true == isError)
    {
        ESP_LOGE(TAG, "Setup failed. Execution stopped.");

        /* Brick */
        while(1)
        {
            ;
        }
    }
    else
    {
        ESP_LOGI(TAG, "Setup finished.");
    }
}

/**
 * Main loop, which is called periodically.
 */
void loop()
{
    uint32_t        idx             = 0;
    unsigned long   timestampEnd    = 0;
    unsigned long   timestampBegin  = 0;
    unsigned long   delta           = 0;

    /* ---------- */

    ESP_LOGI(TAG, "Creating directories and files ...");
    ESP_LOGI(TAG, "Directories per level: %u", DIR_CNT);
    ESP_LOGI(TAG, "Directory depth      : %u", DIR_DEPTH);
    ESP_LOGI(TAG, "Files per directory  : %u", FILE_CNT);
    
    timestampBegin  = millis();
    for(idx = 0; idx < DIR_CNT; ++idx)
    {
        String dirName  = String("directory_") + idx;

        createDirectoryRecursively(FILESYSTEM, "/", dirName, "file", DIR_DEPTH, FILE_CNT);
    }
    timestampEnd = millis();
    delta = timestampEnd - timestampBegin;
    ESP_LOGI(TAG, "--> Duration: %lu s %lu ms", delta / 1000, delta % 1000);

    /* ---------- */

    ESP_LOGI(TAG, "Walking through directories recursively ...");
    timestampBegin  = millis();
    listRootRecursively(FILESYSTEM, false);
    timestampEnd = millis();
    delta = timestampEnd - timestampBegin;
    ESP_LOGI(TAG, "--> Duration: %lu s %lu ms", delta / 1000, delta % 1000);

    /* ---------- */

    const char* str1 = "Hello World!";
    const char* str2 = "The winter will come.";
    char        tmp[strlen(str1) + strlen(str2) + 1];

    ESP_LOGI(TAG, "sprintf test ...");
    timestampBegin  = millis();
    for(idx = 0; idx < STR_FUNC_CNT; ++idx)
    {
        sprintf(tmp, "%s%s", str1, str2);
    }
    timestampEnd = millis();
    delta = timestampEnd - timestampBegin;
    ESP_LOGI(TAG, "--> Duration: %lu s %lu ms", delta / 1000, delta % 1000);

    /* ---------- */

    ESP_LOGI(TAG, "strcpy/strcat test ...");
    timestampBegin  = millis();
    for(idx = 0; idx < STR_FUNC_CNT; ++idx)
    {
        strcpy(tmp, str1);
        strcat(tmp, str2);
    }
    timestampEnd = millis();
    delta = timestampEnd - timestampBegin;
    ESP_LOGI(TAG, "--> Duration: %lu s %lu ms", delta / 1000, delta % 1000);

    /* ---------- */

    ESP_LOGI(TAG, "by hand test ...");
    timestampBegin  = millis();
    for(idx = 0; idx < STR_FUNC_CNT; ++idx)
    {
        char*       dst = tmp;
        const char* src = str1;

        while(*src != '\0')
        {
            *dst = *src;
            ++src;
            ++dst;
        }

        src = str2;
        while(*src != '\0')
        {
            *dst = *src;
            ++src;
            ++dst;
        }

        *dst = '\0';
    }
    timestampEnd = millis();
    delta = timestampEnd - timestampBegin;
    ESP_LOGI(TAG, "--> Duration: %lu s %lu ms", delta / 1000, delta % 1000);

    /* ---------- */

    ESP_LOGI(TAG, "Reset to restart.");
    while(1)
    {
        ;
    }
}

/******************************************************************************
 * Local functions
 *****************************************************************************/

static void createDirectoryRecursively(fs::FS& fs, const String& path, const String& baseDirName, const String& baseFileName, uint32_t dirDepth, uint32_t fileCnt)
{
    String  dirName     = baseDirName + "_";
    String  fullPath    = path;

    if (0 == dirDepth)
    {
        return;
    }

    --dirDepth;
    dirName += dirDepth;
    fullPath += dirName;

    //ESP_LOGI(TAG, "Creating directory \"%s\".", fullPath.c_str());

    if (false == fs.mkdir(fullPath))
    {
        ESP_LOGE(TAG, "Creating directory \"%s\" failed.", fullPath.c_str());
    }
    else
    {
        uint32_t fileIndex;

        fullPath += "/";

        /* Create files in previously created directory. */
        for(fileIndex = 0; fileIndex < fileCnt; ++fileIndex)
        {
            String  fullPathFileName    = fullPath + baseFileName + "_";
            File    fd;

            fullPathFileName += fileIndex;
            
            //ESP_LOGI(TAG, "Creating file \"%s\".", fullPathFileName.c_str());
            fd = fs.open(fullPathFileName, "w");

            if (false == fd)
            {
                ESP_LOGE(TAG, "Creating file \"%s\" failed.", fullPathFileName.c_str());
            }
            else
            {
                fd.printf("%s", fullPathFileName.c_str());
                fd.close();
            }
        }

        if (0 < dirDepth)
        {
            createDirectoryRecursively(fs, fullPath, baseDirName, baseFileName, dirDepth, fileCnt);
        }
    }
}

static void listRecursively(File& path, bool show)
{
    File fd = path.openNextFile();

    while(true == fd)
    {
        if (true == fd.isDirectory())
        {
            if (true == show)
            {
                ESP_LOGI(TAG, "Enter directory \"%s\"", fd.path());
            }

            listRecursively(fd, show);

            if (true == show)
            {
                ESP_LOGI(TAG, "Leave directory \"%s\"", fd.path());
            }
        }
        else if (true == show)
        {
            ESP_LOGI(TAG, "\"%s\"", fd.path());
        }
        else
        {
            ;
        }

        fd.close();
        fd = path.openNextFile();
    }
}

static void listRootRecursively(fs::FS& fs, bool show)
{
    File fdRoot = fs.open("/", "r");

    if (false == fdRoot)
    {
        ESP_LOGE(TAG, "Failed to open root directory.");
    }
    else
    {
        listRecursively(fdRoot, show);
    }
}
