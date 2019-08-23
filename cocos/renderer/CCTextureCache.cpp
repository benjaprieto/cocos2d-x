/****************************************************************************
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2011      Zynga Inc.
Copyright (c) 2013-2016 Chukong Technologies Inc.
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

#include "renderer/CCTextureCache.h"

#include <errno.h>
#include <stack>
#include <cctype>
#include <list>

#include "renderer/CCTexture2D.h"
#include "base/ccMacros.h"
#include "base/ccUTF8.h"
#include "base/CCDirector.h"
#include "base/CCScheduler.h"
#include "platform/CCFileUtils.h"
#include "base/ccUtils.h"
#include "base/CCNinePatchImageParser.h"

// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
#ifdef LINUX
#include <fcntl.h>
#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"
#include "../../../ExternalLibraries/CoreMobile/LibCoreMobile/Utils/md5.h"
#endif
// CROWDSTAR_COCOSPATCH_END

using namespace std;

NS_CC_BEGIN

std::string TextureCache::s_etc1AlphaFileSuffix = "@alpha";

// implementation TextureCache

void TextureCache::setETC1AlphaFileSuffix(const std::string& suffix)
{
    s_etc1AlphaFileSuffix = suffix;
}

std::string TextureCache::getETC1AlphaFileSuffix()
{
    return s_etc1AlphaFileSuffix;
}

TextureCache * TextureCache::getInstance()
{
    return Director::getInstance()->getTextureCache();
}

TextureCache::TextureCache()
: _loadingThread(nullptr)
, _needQuit(false)
, _asyncRefCount(0)
{
}

TextureCache::~TextureCache()
{
    CCLOGINFO("deallocing TextureCache: %p", this);

    for (auto& texture : _textures)
        texture.second->release();

    CC_SAFE_DELETE(_loadingThread);
}

void TextureCache::destroyInstance()
{
}

TextureCache * TextureCache::sharedTextureCache()
{
    return Director::getInstance()->getTextureCache();
}

void TextureCache::purgeSharedTextureCache()
{
}

std::string TextureCache::getDescription() const
{
    return StringUtils::format("<TextureCache | Number of textures = %d>", static_cast<int>(_textures.size()));
}

struct TextureCache::AsyncStruct
{
public:
    AsyncStruct
// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
// Code was:
//    ( const std::string& fn,const std::function<void(Texture2D*)>& f,
//      const std::string& key )
//
    ( const std::string& fn,const std::function<void(Texture2D*, std::string)>& f,
      const std::string& key, Ref* t = nullptr )
// CROWDSTAR_COCOSPATCH_END
      : filename(fn), callback(f),callbackKey( key ),
        pixelFormat(Texture2D::getDefaultAlphaPixelFormat()),
        loadSuccess(false)
// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
		,target(t)
// CROWDSTAR_COCOSPATCH_END
    {}

    std::string filename;
// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
// Added string parameter
    std::function<void(Texture2D*, std::string)> callback;
// CROWDSTAR_COCOSPATCH_END
    std::string callbackKey;
    Image image;
    Image imageAlpha;
    Texture2D::PixelFormat pixelFormat;
    bool loadSuccess;
    
// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
    Ref* target;
// CROWDSTAR_COCOSPATCH_END

};

/**
 The addImageAsync logic follow the steps:
 - find the image has been add or not, if not add an AsyncStruct to _requestQueue  (GL thread)
 - get AsyncStruct from _requestQueue, load res and fill image data to AsyncStruct.image, then add AsyncStruct to _responseQueue (Load thread)
 - on schedule callback, get AsyncStruct from _responseQueue, convert image to texture, then delete AsyncStruct (GL thread)

 the Critical Area include these members:
 - _requestQueue: locked by _requestMutex
 - _responseQueue: locked by _responseMutex

 the object's life time:
 - AsyncStruct: construct and destruct in GL thread
 - image data: new in Load thread, delete in GL thread(by Image instance)

 Note:
 - all AsyncStruct referenced in _asyncStructQueue, for unbind function use.

 How to deal add image many times?
 - At first, this situation is abnormal, we only ensure the logic is correct.
 - If the image has been loaded, the after load image call will return immediately.
 - If the image request is in queue already, there will be more than one request in queue,
 - In addImageAsyncCallback, will deduplicate the request to ensure only create one texture.

 Does process all response in addImageAsyncCallback consume more time?
 - Convert image to texture faster than load image from disk, so this isn't a
 problem.

 Call unbindImageAsync(path) to prevent the call to the callback when the
 texture is loaded.
 */

// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
// Was:
// void TextureCache::addImageAsync(const std::string &path, const std::function<void(Texture2D*)>& callback)
// {
//    addImageAsync( path, callback, path );
// }
void TextureCache::addImageAsync(const std::string &path, const std::function<void(Texture2D*, std::string)>& callback)
{
    addImageAsync( path, callback, path, nullptr );
}
// CROWDSTAR_COCOSPATCH_END

/**
 The addImageAsync logic follow the steps:
 - find the image has been add or not, if not add an AsyncStruct to _requestQueue  (GL thread)
 - get AsyncStruct from _requestQueue, load res and fill image data to AsyncStruct.image, then add AsyncStruct to _responseQueue (Load thread)
 - on schedule callback, get AsyncStruct from _responseQueue, convert image to texture, then delete AsyncStruct (GL thread)
 
 the Critical Area include these members:
 - _requestQueue: locked by _requestMutex
 - _responseQueue: locked by _responseMutex
 
 the object's life time:
 - AsyncStruct: construct and destruct in GL thread
 - image data: new in Load thread, delete in GL thread(by Image instance)
 
 Note:
 - all AsyncStruct referenced in _asyncStructQueue, for unbind function use.
 
 How to deal add image many times?
 - At first, this situation is abnormal, we only ensure the logic is correct.
 - If the image has been loaded, the after load image call will return immediately.
 - If the image request is in queue already, there will be more than one request in queue,
 - In addImageAsyncCallback, will deduplicate the request to ensure only create one texture.
 
 Does process all response in addImageAsyncCallback consume more time?
 - Convert image to texture faster than load image from disk, so this isn't a
 problem.

 The callbackKey allows to unbind the callback in cases where the loading of
 path is requested by several sources simultaneously. Each source can then
 unbind the callback independently as needed whilst a call to
 unbindImageAsync(path) would be ambiguous.
 */
// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
// Was:
// void TextureCache::addImageAsync(const std::string &path, const std::function<void(Texture2D*)>& callback, const std::string& callbackKey)
//
//
void TextureCache::addImageAsync(const std::string &path, const std::function<void(Texture2D*, std::string)>& callback, const std::string& callbackKey, Ref* target)
// CROWDSTAR_COCOSPATCH_END
{
    Texture2D *texture = nullptr;

    std::string fullpath = FileUtils::getInstance()->fullPathForFilename(path);

    auto it = _textures.find(fullpath);
    if (it != _textures.end())
        texture = it->second;

    if (texture != nullptr)
    {
// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
// Was:
//        if (callback) callback(texture);
//
        if (callback) callback(texture, path);
// CROWDSTAR_COCOSPATCH_END
        return;
    }

    // check if file exists
    if (fullpath.empty() || !FileUtils::getInstance()->isFileExist(fullpath)) {
// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
// Was:
//        if (callback) callback(nullptr);
//
        if (callback) callback(nullptr, "");
// CROWDSTAR_COCOSPATCH_END
        return;
    }

    // lazy init
    if (_loadingThread == nullptr)
    {
        // create a new thread to load images
        _needQuit = false;
        _loadingThread = new (std::nothrow) std::thread(&TextureCache::loadImage, this);
    }

    if (0 == _asyncRefCount)
    {
        Director::getInstance()->getScheduler()->schedule(CC_SCHEDULE_SELECTOR(TextureCache::addImageAsyncCallBack), this, 0, false);
    }

    ++_asyncRefCount;

    // generate async struct
    AsyncStruct *data =
// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
// Was:
//      new (std::nothrow) AsyncStruct(fullpath, callback, callbackKey);
//
      new (std::nothrow) AsyncStruct(fullpath, callback, callbackKey, target);
// CROWDSTAR_COCOSPATCH_END
    
    // add async struct into queue
    _asyncStructQueue.push_back(data);
    std::unique_lock<std::mutex> ul(_requestMutex);
    _requestQueue.push_back(data);
    _sleepCondition.notify_one();
}

// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
// Was:
// void TextureCache::unbindImageAsync(const std::string &callbackKey)
// {
//
void TextureCache::unbindImageAsync(const std::string &callbackKey, Ref* target)
{
    bool bFound = false;
// CROWDSTAR_COCOSPATCH_END
    
    if (_asyncStructQueue.empty())
    {
        return;
    }

    for (auto& asyncStruct : _asyncStructQueue)
    {
// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
// Was:
//        if (asyncStruct->callbackKey == callbackKey)
//
        if (asyncStruct->callbackKey == callbackKey && asyncStruct->target == target)
// CROWDSTAR_COCOSPATCH_END
        {
            asyncStruct->callback = nullptr;
            
// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
            asyncStruct->target = nullptr;
            bFound = true;
// CROWDSTAR_COCOSPATCH_END
            
        }
    }
    
// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
    if (!bFound)
    {
        _limboUnbindMutex.lock();
        _limboUnbindFiles.insert(callbackKey);
        _limboUnbindMutex.unlock();
    }
// CROWDSTAR_COCOSPATCH_END
}

void TextureCache::unbindAllImageAsync()
{
    if (_asyncStructQueue.empty())
    {
        return;

    }
    for (auto& asyncStruct : _asyncStructQueue)
    {
        asyncStruct->callback = nullptr;
// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
        asyncStruct->target = nullptr;
// CROWDSTAR_COCOSPATCH_END
    }
}

void TextureCache::loadImage()
{
    AsyncStruct *asyncStruct = nullptr;
    while (!_needQuit)
    {
        std::unique_lock<std::mutex> ul(_requestMutex);
        // pop an AsyncStruct from request queue
        if (_requestQueue.empty())
        {
            asyncStruct = nullptr;
        }
        else
        {
            asyncStruct = _requestQueue.front();
            _requestQueue.pop_front();
        }

        if (nullptr == asyncStruct) {
            if (_needQuit) {
                break;
            }
            _sleepCondition.wait(ul);
            continue;
        }
        ul.unlock();

        // load image
        asyncStruct->loadSuccess = asyncStruct->image.initWithImageFileThreadSafe(asyncStruct->filename);

        // ETC1 ALPHA supports.
        if (asyncStruct->loadSuccess && asyncStruct->image.getFileType() == Image::Format::ETC && !s_etc1AlphaFileSuffix.empty())
        { // check whether alpha texture exists & load it
            auto alphaFile = asyncStruct->filename + s_etc1AlphaFileSuffix;
            if (FileUtils::getInstance()->isFileExist(alphaFile))
                asyncStruct->imageAlpha.initWithImageFileThreadSafe(alphaFile);
        }
        // push the asyncStruct to response queue
        _responseMutex.lock();
        _responseQueue.push_back(asyncStruct);
        _responseMutex.unlock();
    }
}

void TextureCache::addImageAsyncCallBack(float /*dt*/)
{
    Texture2D *texture = nullptr;
    AsyncStruct *asyncStruct = nullptr;
    while (true)
    {
        // pop an AsyncStruct from response queue
        _responseMutex.lock();
        if (_responseQueue.empty())
        {
            asyncStruct = nullptr;
        }
        else
        {
            asyncStruct = _responseQueue.front();
            _responseQueue.pop_front();

            // the asyncStruct's sequence order in _asyncStructQueue must equal to the order in _responseQueue
            CC_ASSERT(asyncStruct == _asyncStructQueue.front());
            _asyncStructQueue.pop_front();
        }
        _responseMutex.unlock();

        if (nullptr == asyncStruct) {
            break;
        }

        // check the image has been convert to texture or not
        auto it = _textures.find(asyncStruct->filename);
        if (it != _textures.end())
        {
            texture = it->second;
        }
        else
        {
            // convert image to texture
            if (asyncStruct->loadSuccess)
            {
                Image* image = &(asyncStruct->image);
                // generate texture in render thread
                texture = new (std::nothrow) Texture2D();

                texture->initWithImage(image, asyncStruct->pixelFormat);
                //parse 9-patch info
                this->parseNinePatchImage(image, texture, asyncStruct->filename);
#if CC_ENABLE_CACHE_TEXTURE_DATA
                // cache the texture file name
                VolatileTextureMgr::addImageTexture(texture, asyncStruct->filename);
#endif
                // cache the texture. retain it, since it is added in the map
                _textures.emplace(asyncStruct->filename, texture);
                texture->retain();

                texture->autorelease();
                // ETC1 ALPHA supports.
                if (asyncStruct->imageAlpha.getFileType() == Image::Format::ETC) {
                    auto alphaTexture = new(std::nothrow) Texture2D();
                    if(alphaTexture != nullptr && alphaTexture->initWithImage(&asyncStruct->imageAlpha, asyncStruct->pixelFormat)) {
                        texture->setAlphaTexture(alphaTexture);
                    }
                    CC_SAFE_RELEASE(alphaTexture);
                }
            }
            else {
                texture = nullptr;
                CCLOG("cocos2d: failed to call TextureCache::addImageAsync(%s)", asyncStruct->filename.c_str());
            }
        }

        // call callback function
        if (asyncStruct->callback)
        {
// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
// Was:
//
//             (asyncStruct->callback)(texture);
//
            _limboUnbindMutex.lock();
            bool bFound = (_limboUnbindFiles.erase(asyncStruct->filename) == 1);
            _limboUnbindMutex.unlock();
            if (!bFound)
            {
				(asyncStruct->callback)(texture, asyncStruct->filename);
			}
// CROWDSTAR_COCOSPATCH_END
        }

        // release the asyncStruct
        delete asyncStruct;
        --_asyncRefCount;
    }

    if (0 == _asyncRefCount)
    {
        Director::getInstance()->getScheduler()->unschedule(CC_SCHEDULE_SELECTOR(TextureCache::addImageAsyncCallBack), this);
    }
}

// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
// Was:
// Texture2D * TextureCache::addImage(const std::string &path)
//
Texture2D * TextureCache::addImage(const std::string &path, bool useAlpha, int parentGarmentVersion)
// CROWDSTAR_COCOSPATCH_END
{
    Texture2D * texture = nullptr;
    Image* image = nullptr;
    // Split up directory and filename
    // MUTEX:
    // Needed since addImageAsync calls this method from a different thread

    std::string fullpath = FileUtils::getInstance()->fullPathForFilename(path);

// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
#ifdef LINUX
    string assetsPath = FileUtils::getInstance()->getAssetsPath();
    bool isAssetsFile = fullpath.find(assetsPath) != string::npos;
    
    string relPath;
    if (isAssetsFile)
    {
        relPath = path.substr(path.find("/") + 1);
        relPath = relPath.substr(relPath.find("/") + 1);
        relPath = relPath.substr(relPath.find("/") + 1);
        
        fullpath = assetsPath + relPath;
    }
#endif
// CROWDSTAR_COCOSPATCH_END
    
    if (fullpath.size() == 0)
    {
        return nullptr;
    }
    auto it = _textures.find(fullpath);
    if (it != _textures.end())
        texture = it->second;

    if (!texture)
    {
        // all images are handled by UIImage except PVR extension that is handled by our own handler
        do
        {
            image = new (std::nothrow) Image();
            CC_BREAK_IF(nullptr == image);

// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
// Was:
//            bool bRet = image->initWithImageFile(fullpath);
//            CC_BREAK_IF(!bRet);
            
#ifndef LINUX
            bool bRet = image->initWithImageFile(fullpath, useAlpha);
            CC_BREAK_IF(!bRet);
#else
            string imageFileMd5;
            do
            {
                FILE *imageFile = fopen(fullpath.c_str(), "rb");
                if (!imageFile)
                {
                    CCLOG("Couldn't open the image file. [%s]", fullpath.c_str());
                    break;
                }
                
                fseek(imageFile, 0, SEEK_END);
                long imageFileSize = ftell(imageFile);
                rewind(imageFile);
                
                char *imageFileData = (char *) malloc(sizeof(char) * imageFileSize);
                if (!imageFileData)
                {
                    CCLOG("Couldn't allocate memory for the image file.");
                    fclose(imageFile);
                    break;
                }
                
                size_t result = fread(imageFileData, sizeof(char), imageFileSize, imageFile);
                if (result != imageFileSize)
                {
                    CCLOG("Couldn't load the image file into memory.");
                    fclose(imageFile);
                    free(imageFileData);
                    break;
                }
                
                fclose(imageFile);
                
                MD5 md5;
                md5.update(imageFileData, imageFileSize);
                md5.finalize();
                imageFileMd5 = md5.hexdigest();
                
                free(imageFileData);
            } while (0);
            
            string decodedImageFilePath = fullpath + "_raw";
            if (isAssetsFile)
            {
                decodedImageFilePath = FileUtils::getInstance()->getRawAssetsPath() + relPath + "_raw";
            }
            
            struct flock fl = { F_RDLCK, SEEK_SET, 0, 0, 0 };
            bool bRet = false;
            bool decodedImageExists = false;
            do
            {
                FILE *decodedImageFile = fopen(decodedImageFilePath.c_str(), "rb");
                if (!decodedImageFile)
                {
                    CCLOG("Couldn't open the decoded image file. [%s]", decodedImageFilePath.c_str());
                    break;
                }
                
                fl.l_type = F_RDLCK;
                fcntl(fileno(decodedImageFile), F_SETLKW, &fl);
                
                fseek(decodedImageFile, 0, SEEK_END);
                long decodedImageFileSize = ftell(decodedImageFile);
                rewind(decodedImageFile);
                
                CCLOG("Decoded image file size: %ld", decodedImageFileSize);
                
                char *decodedImageFileData = (char *) malloc(sizeof(char) * decodedImageFileSize);
                if (!decodedImageFileData)
                {
                    CCLOG("Couldn't allocate memory for the decoded image file.");
                    
                    fl.l_type = F_UNLCK;
                    fcntl(fileno(decodedImageFile), F_SETLKW, &fl);
                    
                    fclose(decodedImageFile);
                    break;
                }
                
                size_t result = fread(decodedImageFileData, sizeof(char), decodedImageFileSize, decodedImageFile);
                if (result != decodedImageFileSize)
                {
                    CCLOG("Couldn't load the decoded image file into memory.");
                    
                    fl.l_type = F_UNLCK;
                    fcntl(fileno(decodedImageFile), F_SETLKW, &fl);
                    
                    fclose(decodedImageFile);
                    free(decodedImageFileData);
                    break;
                }
                
                string imageManifestMd5;
                string md5ManifestFilePath = FileUtils::getInstance()->getRawAssetsPath() + string("md5_manifest.json");
                FILE *md5ManifestFile = fopen(md5ManifestFilePath.c_str(), "r");
                if (md5ManifestFile)
                {
                    fl.l_type = F_RDLCK;
                    fcntl(fileno(md5ManifestFile), F_SETLKW, &fl);
                    
                    fseek(md5ManifestFile, 0, SEEK_END);
                    long md5ManifestFileSize = ftell(md5ManifestFile) + 1;
                    rewind(md5ManifestFile);
                    
                    char md5ManifestJson[md5ManifestFileSize];
                    if (fgets(md5ManifestJson, static_cast<int>(md5ManifestFileSize), md5ManifestFile))
                    {
                        rapidjson::Document doc;
                        if (!doc.Parse<rapidjson::kParseDefaultFlags>(md5ManifestJson).HasParseError())
                        {
                            if (doc.HasMember(path.c_str()))
                            {
                                imageManifestMd5 = doc[path.c_str()].GetString();
                            }
                        }
                        else
                        {
                            CCLOG("%s parse error: %s", md5ManifestFilePath.c_str(), doc.GetParseError());
                            remove(md5ManifestFilePath.c_str());
                        }
                    }
                    
                    fl.l_type = F_UNLCK;
                    fcntl(fileno(md5ManifestFile), F_SETLKW, &fl);
                    
                    fclose(md5ManifestFile);
                }
                
                if (imageManifestMd5 != imageFileMd5)
                {
                    remove(decodedImageFilePath.c_str());
                    
                    fl.l_type = F_UNLCK;
                    fcntl(fileno(decodedImageFile), F_SETLKW, &fl);
                    
                    fclose(decodedImageFile);
                    free(decodedImageFileData);
                    break;
                }
                
                fl.l_type = F_UNLCK;
                fcntl(fileno(decodedImageFile), F_SETLKW, &fl);
                
                fclose(decodedImageFile);
                
                DecodedImageHeader *decodedImageHeader = (DecodedImageHeader *) decodedImageFileData;
                const char *decodedImageData = decodedImageFileData + sizeof(DecodedImageHeader);
                bRet = image->initWithRawData(reinterpret_cast<const unsigned char *>(decodedImageData), strlen(decodedImageData), decodedImageHeader->width, decodedImageHeader->height, 8);
                
                free(decodedImageFileData);
                decodedImageExists = true;
            } while (0);
            
            if (!bRet)
            {
                bRet = image->initWithImageFile(fullpath, useAlpha);
                CC_BREAK_IF(!bRet);
            }
            
            if (!decodedImageExists)
            {
                DecodedImageHeader decodedImageHeader;
                decodedImageHeader.width = (uint16_t) image->getWidth();
                decodedImageHeader.height = (uint16_t) image->getHeight();
                decodedImageHeader.version = (uint32_t) parentGarmentVersion;
                
                size_t decodedImageFileSize = image->getDataLen() + sizeof(DecodedImageHeader);
                char *decodedImageFileData = (char *) malloc(decodedImageFileSize);
                memset(decodedImageFileData, 0, decodedImageFileSize);
                
                char *writePtr = decodedImageFileData;
                
                memcpy(writePtr, &decodedImageHeader, sizeof(DecodedImageHeader));
                writePtr += sizeof(DecodedImageHeader);
                
                memcpy(writePtr, reinterpret_cast<const char *>(image->getData()), image->getDataLen());
                
                bool decodedImageCreated = false;
                do
                {
                    FileUtils::getInstance()->createDirectory(decodedImageFilePath.substr(0, decodedImageFilePath.rfind("/")));
                    FILE *decodedImageFile = fopen(decodedImageFilePath.c_str(), "wb");
                    if (!decodedImageFile)
                    {
                        CCLOG("Error creating file!");
                        break;
                    }
                    
                    fl.l_type = F_WRLCK;
                    fcntl(fileno(decodedImageFile), F_SETLKW, &fl);
                    
                    long bytesWritten = fwrite(decodedImageFileData, 1, decodedImageFileSize, decodedImageFile);
                    
                    fl.l_type = F_UNLCK;
                    fcntl(fileno(decodedImageFile), F_SETLKW, &fl);
                    
                    fclose(decodedImageFile);
                    
                    if (bytesWritten != decodedImageFileSize)
                    {
                        CCLOG("failed to write all data!");
                        break;
                    }
                    
                    decodedImageCreated = true;
                } while (0);
                
                free(decodedImageFileData);
                
                if (decodedImageCreated && !imageFileMd5.empty())
                {
                    do
                    {
                        string md5ManifestFilePath = FileUtils::getInstance()->getRawAssetsPath() + string("md5_manifest.json");
                        FILE *md5ManifestFile = fopen(md5ManifestFilePath.c_str(), "r");
                        if (md5ManifestFile)
                        {
                            fl.l_type = F_RDLCK;
                            fcntl(fileno(md5ManifestFile), F_SETLKW, &fl);
                            
                            fseek(md5ManifestFile, 0, SEEK_END);
                            long md5ManifestFileSize = ftell(md5ManifestFile) + 1;
                            rewind(md5ManifestFile);
                            
                            char md5ManifestJson[md5ManifestFileSize];
                            if (fgets(md5ManifestJson, static_cast<int>(md5ManifestFileSize), md5ManifestFile))
                            {
                                rapidjson::Document doc;
                                if (!doc.Parse<rapidjson::kParseDefaultFlags>(md5ManifestJson).HasParseError())
                                {
                                    fl.l_type = F_UNLCK;
                                    fcntl(fileno(md5ManifestFile), F_SETLKW, &fl);
                                    
                                    fclose(md5ManifestFile);
                                    
                                    if (doc.HasMember(path.c_str()))
                                    {
                                        doc.RemoveMember(path.c_str());
                                    }
                                    
                                    doc.AddMember(path.c_str(), imageFileMd5.c_str(), doc.GetAllocator());
                                    
                                    md5ManifestFile = fopen(md5ManifestFilePath.c_str(), "w");
                                    
                                    fl.l_type = F_WRLCK;
                                    fcntl(fileno(md5ManifestFile), F_SETLKW, &fl);
                                    
                                    rapidjson::StringBuffer buffer;
                                    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                                    doc.Accept(writer);
                                    
                                    char *md5ManifestData = strdup(buffer.GetString());
                                    fputs(md5ManifestData, md5ManifestFile);
                                    free(md5ManifestData);
                                }
                                else
                                {
                                    CCLOG("%s parse error: %s", md5ManifestFilePath.c_str(), doc.GetParseError());
                                    remove(md5ManifestFilePath.c_str());
                                }
                            }
                        }
                        else
                        {
                            rapidjson::Document doc;
                            doc.SetObject();
                            doc.AddMember(path.c_str(), imageFileMd5.c_str(), doc.GetAllocator());
                            
                            md5ManifestFile = fopen(md5ManifestFilePath.c_str(), "w");
                            
                            fl.l_type = F_WRLCK;
                            fcntl(fileno(md5ManifestFile), F_SETLKW, &fl);
                            
                            rapidjson::StringBuffer buffer;
                            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                            doc.Accept(writer);
                            
                            char *md5ManifestData = strdup(buffer.GetString());
                            fputs(md5ManifestData, md5ManifestFile);
                            free(md5ManifestData);
                        }
                        
                        fl.l_type = F_UNLCK;
                        fcntl(fileno(md5ManifestFile), F_SETLKW, &fl);
                        
                        fclose(md5ManifestFile);
                    } while (0);
                }
            }
#endif
// CROWDSTAR_COCOSPATCH_END

            texture = new (std::nothrow) Texture2D();

            if (texture && texture->initWithImage(image))
            {
#if CC_ENABLE_CACHE_TEXTURE_DATA
                // cache the texture file name
                VolatileTextureMgr::addImageTexture(texture, fullpath);
#endif
                // texture already retained, no need to re-retain it
                _textures.emplace(fullpath, texture);

                //-- ANDROID ETC1 ALPHA SUPPORTS.
                std::string alphaFullPath = path + s_etc1AlphaFileSuffix;
                if (image->getFileType() == Image::Format::ETC && !s_etc1AlphaFileSuffix.empty() && FileUtils::getInstance()->isFileExist(alphaFullPath))
                {
                    Image alphaImage;
                    if (alphaImage.initWithImageFile(alphaFullPath))
                    {
                        Texture2D *pAlphaTexture = new(std::nothrow) Texture2D;
                        if(pAlphaTexture != nullptr && pAlphaTexture->initWithImage(&alphaImage)) {
                            texture->setAlphaTexture(pAlphaTexture);
                        }
                        CC_SAFE_RELEASE(pAlphaTexture);
                    }
                }

                //parse 9-patch info
                this->parseNinePatchImage(image, texture, path);
            }
            else
            {
                CCLOG("cocos2d: Couldn't create texture for file:%s in TextureCache", path.c_str());
                CC_SAFE_RELEASE(texture);
                texture = nullptr;
            }
        } while (0);
    }

    CC_SAFE_RELEASE(image);

    return texture;
}

void TextureCache::parseNinePatchImage(cocos2d::Image *image, cocos2d::Texture2D *texture, const std::string& path)
{
    if (NinePatchImageParser::isNinePatchImage(path))
    {
        Rect frameRect = Rect(0, 0, image->getWidth(), image->getHeight());
        NinePatchImageParser parser(image, frameRect, false);
        texture->addSpriteFrameCapInset(nullptr, parser.parseCapInset());
    }

}

Texture2D* TextureCache::addImage(Image *image, const std::string &key)
{
    CCASSERT(image != nullptr, "TextureCache: image MUST not be nil");
    CCASSERT(image->getData() != nullptr, "TextureCache: image MUST not be nil");

    Texture2D * texture = nullptr;

    do
    {
        auto it = _textures.find(key);
        if (it != _textures.end()) {
            texture = it->second;
            break;
        }

        texture = new (std::nothrow) Texture2D();

        if (texture)
        {
            if (texture->initWithImage(image))
            {
                _textures.emplace(key, texture);
            }
            else
            {
                CC_SAFE_RELEASE(texture);
                texture = nullptr;
                CCLOG("cocos2d: initWithImage failed!");
            }
        }
        else
        {
            CCLOG("cocos2d: Allocating memory for Texture2D failed!");
        }

    } while (0);

#if CC_ENABLE_CACHE_TEXTURE_DATA
    VolatileTextureMgr::addImage(texture, image);
#endif

    return texture;
}

bool TextureCache::reloadTexture(const std::string& fileName)
{
    Texture2D * texture = nullptr;
    Image * image = nullptr;

    std::string fullpath = FileUtils::getInstance()->fullPathForFilename(fileName);
    if (fullpath.size() == 0)
    {
        return false;
    }

    auto it = _textures.find(fullpath);
    if (it != _textures.end()) {
        texture = it->second;
    }

    bool ret = false;
    if (!texture) {
        texture = this->addImage(fullpath);
        ret = (texture != nullptr);
    }
    else
    {
        do {
            image = new (std::nothrow) Image();
            CC_BREAK_IF(nullptr == image);

            bool bRet = image->initWithImageFile(fullpath);
            CC_BREAK_IF(!bRet);

            ret = texture->initWithImage(image);
        } while (0);
    }

    CC_SAFE_RELEASE(image);

    return ret;
}

// TextureCache - Remove

void TextureCache::removeAllTextures()
{
    for (auto& texture : _textures) {
        texture.second->release();
    }
    _textures.clear();
}

void TextureCache::removeUnusedTextures()
{
    for (auto it = _textures.cbegin(); it != _textures.cend(); /* nothing */) {
        Texture2D *tex = it->second;
        if (tex->getReferenceCount() == 1) {
            CCLOG("cocos2d: TextureCache: removing unused texture: %s", it->first.c_str());

            tex->release();
            it = _textures.erase(it);
        }
        else {
            ++it;
        }

    }
}

void TextureCache::removeTexture(Texture2D* texture)
{
    if (!texture)
    {
        return;
    }

    for (auto it = _textures.cbegin(); it != _textures.cend(); /* nothing */) {
        if (it->second == texture) {
            
// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
            _limboUnbindMutex.lock();
            _limboUnbindFiles.erase(it->first);
            _limboUnbindMutex.unlock();
// CROWDSTAR_COCOSPATCH_END
            
            it->second->release();
            it = _textures.erase(it);
            break;
        }
        else
            ++it;
    }
}

void TextureCache::removeTextureForKey(const std::string &textureKeyName)
{
    std::string key = textureKeyName;
    auto it = _textures.find(key);

    if (it == _textures.end()) {
        key = FileUtils::getInstance()->fullPathForFilename(textureKeyName);
        it = _textures.find(key);
    }

    if (it != _textures.end()) {

// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
        _limboUnbindMutex.lock();
        _limboUnbindFiles.erase(it->first);
        _limboUnbindMutex.unlock();
// CROWDSTAR_COCOSPATCH_END
        
        it->second->release();
        _textures.erase(it);
    }
}

Texture2D* TextureCache::getTextureForKey(const std::string &textureKeyName) const
{
    std::string key = textureKeyName;
    auto it = _textures.find(key);

    if (it == _textures.end()) {
        key = FileUtils::getInstance()->fullPathForFilename(textureKeyName);
        it = _textures.find(key);
    }

    if (it != _textures.end())
        return it->second;
    return nullptr;
}

void TextureCache::reloadAllTextures()
{
    //will do nothing
    // #if CC_ENABLE_CACHE_TEXTURE_DATA
    //     VolatileTextureMgr::reloadAllTextures();
    // #endif
}

std::string TextureCache::getTextureFilePath(cocos2d::Texture2D* texture) const
{
    for (auto& item : _textures)
    {
        if (item.second == texture)
        {
            return item.first;
            break;
        }
    }
    return "";
}

void TextureCache::waitForQuit()
{
    // notify sub thread to quick
    std::unique_lock<std::mutex> ul(_requestMutex);
    _needQuit = true;
    _sleepCondition.notify_one();
    ul.unlock();
    if (_loadingThread) _loadingThread->join();
}

std::string TextureCache::getCachedTextureInfo() const
{
    std::string buffer;
    char buftmp[4096];

    unsigned int count = 0;
    unsigned int totalBytes = 0;

    for (auto& texture : _textures) {

        memset(buftmp, 0, sizeof(buftmp));


        Texture2D* tex = texture.second;
        unsigned int bpp = tex->getBitsPerPixelForFormat();
        // Each texture takes up width * height * bytesPerPixel bytes.
        auto bytes = tex->getPixelsWide() * tex->getPixelsHigh() * bpp / 8;
        totalBytes += bytes;
        count++;
        snprintf(buftmp, sizeof(buftmp) - 1, "\"%s\" rc=%lu id=%lu %lu x %lu @ %ld bpp => %lu KB\n",
            texture.first.c_str(),
            (long)tex->getReferenceCount(),
            (long)tex->getName(),
            (long)tex->getPixelsWide(),
            (long)tex->getPixelsHigh(),
            (long)bpp,
            (long)bytes / 1024);

        buffer += buftmp;
    }

    snprintf(buftmp, sizeof(buftmp) - 1, "TextureCache dumpDebugInfo: %ld textures, for %lu KB (%.2f MB)\n", (long)count, (long)totalBytes / 1024, totalBytes / (1024.0f*1024.0f));
    buffer += buftmp;

    return buffer;
}

void TextureCache::renameTextureWithKey(const std::string& srcName, const std::string& dstName)
{
    std::string key = srcName;
    auto it = _textures.find(key);

    if (it == _textures.end()) {
        key = FileUtils::getInstance()->fullPathForFilename(srcName);
        it = _textures.find(key);
    }

    if (it != _textures.end()) {
        std::string fullpath = FileUtils::getInstance()->fullPathForFilename(dstName);
        Texture2D* tex = it->second;

        Image* image = new (std::nothrow) Image();
        if (image)
        {
            bool ret = image->initWithImageFile(dstName);
            if (ret)
            {
                tex->initWithImage(image);
                _textures.emplace(fullpath, tex);
                _textures.erase(it);
            }
            CC_SAFE_DELETE(image);
        }
    }
}

#if CC_ENABLE_CACHE_TEXTURE_DATA

std::list<VolatileTexture*> VolatileTextureMgr::_textures;
bool VolatileTextureMgr::_isReloading = false;

VolatileTexture::VolatileTexture(Texture2D *t)
: _texture(t)
, _uiImage(nullptr)
, _cashedImageType(kInvalid)
, _textureData(nullptr)
, _pixelFormat(Texture2D::PixelFormat::RGBA8888)
, _fileName("")
, _hasMipmaps(false)
, _text("")
{
    _texParams.minFilter = GL_LINEAR;
    _texParams.magFilter = GL_LINEAR;
    _texParams.wrapS = GL_CLAMP_TO_EDGE;
    _texParams.wrapT = GL_CLAMP_TO_EDGE;
}

VolatileTexture::~VolatileTexture()
{
    CC_SAFE_RELEASE(_uiImage);
}

void VolatileTextureMgr::addImageTexture(Texture2D *tt, const std::string& imageFileName)
{
    if (_isReloading)
    {
        return;
    }

    VolatileTexture *vt = findVolotileTexture(tt);

    vt->_cashedImageType = VolatileTexture::kImageFile;
    vt->_fileName = imageFileName;
    vt->_pixelFormat = tt->getPixelFormat();
}

void VolatileTextureMgr::addImage(Texture2D *tt, Image *image)
{
    if (tt == nullptr || image == nullptr)
        return;
    
    VolatileTexture *vt = findVolotileTexture(tt);
    image->retain();
    vt->_uiImage = image;
    vt->_cashedImageType = VolatileTexture::kImage;
}

VolatileTexture* VolatileTextureMgr::findVolotileTexture(Texture2D *tt)
{
    VolatileTexture *vt = nullptr;
    for (const auto& texture : _textures)
    {
        VolatileTexture *v = texture;
        if (v->_texture == tt)
        {
            vt = v;
            break;
        }
    }

    if (!vt)
    {
        vt = new (std::nothrow) VolatileTexture(tt);
        _textures.push_back(vt);
    }

    return vt;
}

void VolatileTextureMgr::addDataTexture(Texture2D *tt, void* data, int dataLen, Texture2D::PixelFormat pixelFormat, const Size& contentSize)
{
    if (_isReloading)
    {
        return;
    }

    VolatileTexture *vt = findVolotileTexture(tt);

    vt->_cashedImageType = VolatileTexture::kImageData;
    vt->_textureData = data;
    vt->_dataLen = dataLen;
    vt->_pixelFormat = pixelFormat;
    vt->_textureSize = contentSize;
}

void VolatileTextureMgr::addStringTexture(Texture2D *tt, const char* text, const FontDefinition& fontDefinition)
{
    if (_isReloading)
    {
        return;
    }

    VolatileTexture *vt = findVolotileTexture(tt);

    vt->_cashedImageType = VolatileTexture::kString;
    vt->_text = text;
    vt->_fontDefinition = fontDefinition;
}

void VolatileTextureMgr::setHasMipmaps(Texture2D *t, bool hasMipmaps)
{
    VolatileTexture *vt = findVolotileTexture(t);
    vt->_hasMipmaps = hasMipmaps;
}

void VolatileTextureMgr::setTexParameters(Texture2D *t, const Texture2D::TexParams &texParams)
{
    VolatileTexture *vt = findVolotileTexture(t);

    if (texParams.minFilter != GL_NONE)
        vt->_texParams.minFilter = texParams.minFilter;
    if (texParams.magFilter != GL_NONE)
        vt->_texParams.magFilter = texParams.magFilter;
    if (texParams.wrapS != GL_NONE)
        vt->_texParams.wrapS = texParams.wrapS;
    if (texParams.wrapT != GL_NONE)
        vt->_texParams.wrapT = texParams.wrapT;
}

void VolatileTextureMgr::removeTexture(Texture2D *t)
{
    for (auto& item : _textures)
    {
        VolatileTexture *vt = item;
        if (vt->_texture == t)
        {
            _textures.remove(vt);
            delete vt;
            break;
        }
    }
}

void VolatileTextureMgr::reloadAllTextures()
{
    _isReloading = true;

    // we need to release all of the glTextures to avoid collisions of texture id's when reloading the textures onto the GPU
    for (auto& item : _textures)
    {
        item->_texture->releaseGLTexture();
    }

    CCLOG("reload all texture");

    for (auto& texture : _textures)
    {
        VolatileTexture *vt = texture;

        switch (vt->_cashedImageType)
        {
        case VolatileTexture::kImageFile:
        {
            reloadTexture(vt->_texture, vt->_fileName, vt->_pixelFormat);

            // etc1 support check whether alpha texture exists & load it
            auto alphaFile = vt->_fileName + TextureCache::getETC1AlphaFileSuffix();
            reloadTexture(vt->_texture->getAlphaTexture(), alphaFile, vt->_pixelFormat);
        }
        break;
        case VolatileTexture::kImageData:
        {
            vt->_texture->initWithData(vt->_textureData,
                vt->_dataLen,
                vt->_pixelFormat,
                vt->_textureSize.width,
                vt->_textureSize.height,
                vt->_textureSize);
        }
        break;
        case VolatileTexture::kString:
        {
            vt->_texture->initWithString(vt->_text.c_str(), vt->_fontDefinition);
        }
        break;
        case VolatileTexture::kImage:
        {
            vt->_texture->initWithImage(vt->_uiImage);
        }
        break;
        default:
            break;
        }
        if (vt->_hasMipmaps) {
            vt->_texture->generateMipmap();
        }
        vt->_texture->setTexParameters(vt->_texParams);
    }

    _isReloading = false;
}

void VolatileTextureMgr::reloadTexture(Texture2D* texture, const std::string& filename, Texture2D::PixelFormat pixelFormat)
{
    if (!texture)
        return;

    Image* image = new (std::nothrow) Image();
    Data data = FileUtils::getInstance()->getDataFromFile(filename);

    if (image && image->initWithImageData(data.getBytes(), data.getSize()))
        texture->initWithImage(image, pixelFormat);

    CC_SAFE_RELEASE(image);
}

#endif // CC_ENABLE_CACHE_TEXTURE_DATA

// CROWDSTAR_COCOSPATCH_BEGIN(Texture2DExtensions)
unsigned int TextureCache::checkCachedTextureInfo()
{
    unsigned int count = 0;
    unsigned int totalBytes = 0;
    
    for (auto element : _textures)
    {
        Texture2D* tex = element.second;
        unsigned int bpp = tex->bitsPerPixelForFormat();
        // Each texture takes up width * height * bytesPerPixel bytes.
        unsigned int bytes = tex->getPixelsWide() * tex->getPixelsHigh() * bpp / 8;
        totalBytes += bytes;
        count++;
    }
    
    CCLOG("cocos2d: CCTextureCache dumpDebugInfo: %ld textures, for %lu KB (%.2f MB)", (long)count, (long)totalBytes / 1024, totalBytes / (1024.0f*1024.0f));
    
    return totalBytes / (1024.0f*1024.0f);
}



void TextureCache::dumpCachedTextureInfo()
{
    unsigned int count = 0;
    unsigned int totalBytes = 0;
    
    for (auto element : _textures)
    {
        Texture2D* tex = element.second;
        unsigned int bpp = tex->bitsPerPixelForFormat();
        // Each texture takes up width * height * bytesPerPixel bytes.
        unsigned int bytes = tex->getPixelsWide() * tex->getPixelsHigh() * bpp / 8;
        totalBytes += bytes;
        count++;
        CCLOG("cocos2d: \"%s\" rc=%lu id=%lu %lu x %lu @ %ld bpp => %lu KB",
              element.first.c_str(),
              (long)tex->getReferenceCount(),
              (long)tex->getName(),
              (long)tex->getPixelsWide(),
              (long)tex->getPixelsHigh(),
              (long)bpp,
              (long)bytes / 1024);
    }
    
    CCLOG("cocos2d: CCTextureCache dumpDebugInfo: %ld textures, for %lu KB (%.2f MB)", (long)count, (long)totalBytes / 1024, totalBytes / (1024.0f*1024.0f));
}
// CROWDSTAR_COCOSPATCH_END

NS_CC_END

