/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#ifndef __LOCALLIZATION_MANAGER_H__
#define __LOCALLIZATION_MANAGER_H__

#include <string>
#include <unordered_map>
// CROWDSTAR_COCOSPATCH_BEGIN(CCBundleJsonReference)
#include "json/document.h"
// CROWDSTAR_COCOSPATCH_END
#include "editor-support/cocostudio/CocosStudioExport.h"

namespace cocostudio {
    /**
    *@brief Localization string manager interface template.
    */
    class CC_STUDIO_DLL ILocalizationManager
    {
    public:
        virtual ~ILocalizationManager() = default;
        virtual bool initLanguageData(std::string file) = 0;
        virtual std::string getLocalizationString(std::string key) = 0;
    };

    /**
    *@brief Localization string manager for output Json data file by cocostudio localization editor.
    *  Note: If changed localization data file manually, please make sure the data file save as
    *    text file format with encoding as 'UTF8 no BOM', otherwise the localization data may
    *    not been parse successfully.
    */
    class CC_STUDIO_DLL JsonLocalizationManager : ILocalizationManager
    {
    public:
        static ILocalizationManager* getInstance();
        static void destroyInstance();

    public:
        /* Init manager with special localize json data file.
        * @param file Name of localize file.
        * @return If manager initialize success return true.
        */
        virtual bool initLanguageData(std::string file);

        /* Get localization string for special key.
        * @param key Special key to search in localization data.
        * @return If manager find the key in localization data, return value
        *  set to key, otherwise return key itself.
        */
        virtual std::string getLocalizationString(std::string key);

    protected:
        JsonLocalizationManager();
        ~JsonLocalizationManager();

    protected:
        rapidjson::Document * languageData;
    };

    class CC_STUDIO_DLL BinLocalizationManager : ILocalizationManager
    {
    public:
        static ILocalizationManager* getInstance();
        static void destroyInstance();

        /* Init manager with special localize binary data file.
        * @param file Name of localize file.
        * @return If manager initialize success return true.
        */
        virtual bool initLanguageData(std::string file);

        /* Get localization string for special key.
        * @param key Special key to search in localization data.
        * @return If manager find the key in localization data, return value
        *  set to key, otherwise return key itself.
        */
        virtual std::string getLocalizationString(std::string key);

    protected:
        BinLocalizationManager();
        ~BinLocalizationManager();

    protected:
        std::unordered_map<std::string, std::string> languageData;
    };

    class CC_STUDIO_DLL LocalizationHelper
    {
    public:
        /* Get current localization manager.
        * @return The instance of current localization manager.
        * If the manager hasn't been set, it will return the singleton instance of BinLocalizationManager.
        */
        static ILocalizationManager* getCurrentManager();

        /* Set current localization manager.
        * @param manager The instance of current manager.
        * @param isBinary Wether the manager is binary localization manager.
        * If the param is false, current manager will be set to JsonLocalizationManager.
        */
        static void setCurrentManager(ILocalizationManager* manager, bool isBinary);

        /* Get the type of current localization manager.
        * @return If current manager is BinLocalizationManager, return true.
        * Otherwise return false, that means current manager is JsonLocalizationManager.
        */
        static bool isBinManager();
    };
}

#endif //__LOCALLIZATION_MANAGER_H__
