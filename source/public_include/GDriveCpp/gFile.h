#pragma once

#include <bitset>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "GDriveCpp/gDrive.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace GDrive {
    struct GDRIVE_API GFileListRequest {
        std::string corpora;
        std::string driveId;
        bool includeItemsFromAllDrives;
        std::string orderBy;
        uint32_t pageSize;
        std::string pageToken;
        std::string q;
        std::string spaces;
        bool supportsAllDrives;
        std::string includePermissionsForView;
        std::string includeLabels;
        std::string fields;
    };

    class GDRIVE_API GFileCapabilities {
      public:
        enum class Type {
            canChangeViewersCanCopyContent = 0,
            canMoveChildrenOutOfDrive = 1,
            canReadDrive = 2,
            canEdit = 3,
            canCopy = 4,
            canComment = 5,
            canAddChildren = 6,
            canDelete = 7,
            canDownload = 8,
            canListChildren = 9,
            canRemoveChildren = 10,
            canRename = 11,
            canTrash = 12,
            canReadRevisions = 13,
            canReadTeamDrive = 14,
            canMoveTeamDriveItem = 15,
            canChangeCopyRequiresWriterPermission = 16,
            canMoveItemIntoTeamDrive = 17,
            canUntrash = 18,
            canModifyContent = 19,
            canMoveItemWithinTeamDrive = 20,
            canMoveItemOutOfTeamDrive = 21,
            canDeleteChildren = 22,
            canMoveChildrenOutOfTeamDrive = 23,
            canMoveChildrenWithinTeamDrive = 24,
            canTrashChildren = 25,
            canMoveItemOutOfDrive = 26,
            canAddMyDriveParent = 27,
            canRemoveMyDriveParent = 28,
            canMoveItemWithinDrive = 29,
            canShare = 30,
            canMoveChildrenWithinDrive = 31,
            canModifyContentRestriction = 32,
            canAddFolderFromAnotherDrive = 33,
            canChangeSecurityUpdateEnabled = 34,
            canAcceptOwnership = 35,
            canReadLabels = 36,
            canModifyLabels = 37,
            canModifyEditorContentRestriction = 38,
            canModifyOwnerContentRestriction = 39,
            canRemoveContentRestriction = 40,
            canDisableInheritedPermissions = 41,
            canEnableInheritedPermissions = 42
        };

      private:
        static std::unordered_map<std::string, GFileCapabilities::Type> _capabilitiesMap;
        std::bitset<86> _capabilities;

      public:
        std::optional<bool> getCapability(const std::string_view& name) const;
        void setCapability(const std::string_view& name, bool value);
    };

    class GDRIVE_API GFile {
      private:
        std::weak_ptr<GCloud::Authentication::OAuthAgent> _client;
        static std::unordered_map<std::string, std::function<void(GFile&, const std::string_view&)>>
            _stringFieldsSetterMap;
        static std::unordered_map<std::string, std::function<void(GFile&, bool)>> _boolFieldsSetterMap;
        static std::unordered_map<std::string, std::function<std::optional<std::string>(GFile&)>>
            _stringFieldsGetterMap;
        static std::unordered_map<std::string, std::function<std::optional<bool>(GFile&)>> _boolFieldsGetterMap;

      public:
        // Primitive types wrapped in std::optional
        std::optional<std::string> kind;
        std::optional<std::string> driveId;
        std::optional<std::string> fileExtension;
        std::optional<bool> copyRequiresWriterPermission;
        std::optional<std::string> md5Checksum;
        // TODO Add ContentHints Nested struct
        std::optional<bool> writersCanShare;
        std::optional<bool> viewedByMe;
        std::optional<std::string> mimeType;
        std::optional<std::unordered_map<std::string, std::string>> exportLinks;
        std::optional<std::vector<std::string>> parents;
        std::optional<std::string> thumbnailLink;
        std::optional<std::string> iconLink;
        std::optional<bool> shared;
        // std::optional<User> lastModifyingUser;
        // std::optional<std::vector<User>> owners;
        std::optional<std::string> headRevisionId;
        // std::optional<User> sharingUser;
        std::optional<std::string> webViewLink;
        std::optional<std::string> webContentLink;
        std::optional<std::string> size;
        std::optional<bool> viewersCanCopyContent;
        // std::optional<std::vector<Permission>> permissions;
        std::optional<bool> hasThumbnail;
        std::optional<std::vector<std::string>> spaces;
        std::optional<std::string> folderColorRgb;
        std::optional<std::string> id;
        std::optional<std::string> name;
        std::optional<std::string> description;
        std::optional<bool> starred;
        std::optional<bool> trashed;
        std::optional<bool> explicitlyTrashed;
        std::optional<std::string> createdTime;
        std::optional<std::string> modifiedTime;
        std::optional<std::string> modifiedByMeTime;
        std::optional<std::string> viewedByMeTime;
        std::optional<std::string> sharedWithMeTime;
        std::optional<std::string> quotaBytesUsed;
        std::optional<std::string> version;
        std::optional<std::string> originalFilename;
        std::optional<bool> ownedByMe;
        std::optional<std::string> fullFileExtension;
        std::optional<std::unordered_map<std::string, std::string>> properties;
        std::optional<std::unordered_map<std::string, std::string>> appProperties;
        std::optional<bool> isAppAuthorized;
        std::optional<std::string> teamDriveId;
        std::optional<GFileCapabilities> capabilities;
        std::optional<bool> hasAugmentedPermissions;
        // std::optional<User> trashingUser;
        std::optional<std::string> thumbnailVersion;
        std::optional<std::string> trashedTime;
        std::optional<bool> modifiedByMe;
        std::optional<std::vector<std::string>> permissionIds;
        // std::optional<ImageMediaMetadata> imageMediaMetadata;
        // std::optional<VideoMediaMetadata> videoMediaMetadata;
        // std::optional<ShortcutDetails> shortcutDetails;
        // std::optional<std::vector<ContentRestriction>> contentRestrictions;
        std::optional<std::string> resourceKey;
        // std::optional<LinkShareMetadata> linkShareMetadata;
        // std::optional<LabelInfo> labelInfo;
        std::optional<std::string> sha1Checksum;
        std::optional<std::string> sha256Checksum;
        std::optional<bool> inheritedPermissionsDisabled;
        GFile(std::weak_ptr<GCloud::Authentication::OAuthAgent> client);

        void setStringField(const std::string_view& field, const std::string_view& value);
        void setBoolField(const std::string_view& field, bool value);

        void print(std::ostream& os);
        void download(const std::string& path = "");
        void upload(const std::string& path);
    };

    class GDRIVE_API GFileList {
      private:
        std::string _nextPageToken;
        std::weak_ptr<GCloud::Authentication::OAuthAgent> _client;

      public:
        static std::optional<GFileList> QueryDirectory(std::weak_ptr<GCloud::Authentication::OAuthAgent> client,
                                                       std::shared_ptr<const GFile> root,
                                                       const std::string& searchPath);
        std::vector<std::shared_ptr<GFile>> files;
        GFileList(std::weak_ptr<GCloud::Authentication::OAuthAgent> client, const GFileListRequest& request);
        void print(std::ostream& os);
    };
}  // namespace GDrive

#ifdef _MSC_VER
#pragma warning(pop)
#endif
