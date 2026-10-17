#include "AppPlatform_android.h"
#include <android/asset_manager.h>
#include <android/native_activity.h>
#include <string>
#include <vector>

class AppPlatform_android23 : public AppPlatform_android
{
	typedef AppPlatform_android super;
public:
	AppPlatform_android23()
	:	_assetManager(NULL)
	{
	}

	BinaryBlob readAssetFile(const std::string& filename) override {
		if (!_isInited)
			return BinaryBlob();

		if (_assetManager != NULL) {
			AAsset* asset = AAssetManager_open(_assetManager, filename.c_str(), AASSET_MODE_BUFFER);
			if (asset != NULL) {
				const int   len = AAsset_getLength(asset);
				const void* buf = len > 0? AAsset_getBuffer(asset) : NULL;

				BinaryBlob blob;
				if (buf != NULL) {
					blob = BinaryBlob(new unsigned char[len], len);
					memcpy(blob.data, buf, len);
				}
				AAsset_close(asset);
				if (blob.data)
					return blob;
			}
		}
		return super::readAssetFile(filename);
	}

	StringVector listLanguageCodes() override {
		StringVector codes;
		if (!_isInited)
			return codes;

		if (_assetManager != NULL) {
			AAssetDir* dir = AAssetManager_openDir(_assetManager, "lang");
			if (dir != NULL) {
				const char* name;
				while ((name = AAssetDir_getNextFileName(dir)) != NULL) {
					std::string n(name);
					if (n.size() > 5 && n.compare(n.size() - 5, 5, ".lang") == 0)
						codes.push_back(n.substr(0, n.size() - 5));
				}
				AAssetDir_close(dir);
			}
		}
		return codes;
	}

	void initWithActivity(struct ANativeActivity* activity) {
		_assetManager = activity->assetManager;
	}

private:
	AAssetManager* _assetManager;
};