#include <cpr/cpr.h>

#include <format>
#include <nlohmann/json.hpp>

#include "GDriveCpp/gDrive.h"
#include "GDriveCpp/gFile.h"
#include "constants.hpp"
#include "logging.hpp"

namespace GDrive {
    std::optional<GFileList> GFileList::QueryDirectory(std::weak_ptr<GCloud::Authentication::OAuthAgent> client,
                                                       std::shared_ptr<const GFile> root,
                                                       const std::string& searchPath) {
        std::filesystem::path path(searchPath);
        std::filesystem::path currentPath;
        std::unique_ptr<GFileList> nextDir;
        for (auto it = path.begin(); it != path.end(); ++it) {
            bool isLast = std::next(it) == path.end();
            if (it->empty() && !isLast) continue;  // Skip empty parts
            if (*it == ".") continue;              // Skip current directory
            if (*it == "..") {
                spdlog::error("Cannot go up directories. Not implemented!");
                return std::nullopt;
            } else {
                if (!root) {
                    nextDir = std::make_unique<GFileList>(
                        client, GFileListRequest{
                                    .corpora = "user",
                                    .includeItemsFromAllDrives = false,
                                    .orderBy = "createdTime desc",
                                    .pageSize = 1,
                                    .q = std::format("name = '{}' and mimeType = 'application/vnd.google-apps.folder'",
                                                     it->generic_string()),
                                    .supportsAllDrives = true,
                                    .fields = "nextPageToken, files(name, id, createdTime)"});

                } else {
                    if (!root->id.has_value()) {
                        spdlog::error("Root file ID is missing, cannot query directory");
                        return std::nullopt;
                    }
                    std::string q = std::format("'{}' in parents", root->id.value());
                    if (!isLast) q += " and mimeType = 'application/vnd.google-apps.folder'";
                    if (!it->empty()) q += std::format(" and name = '{}'", it->generic_string());

                    nextDir = std::make_unique<GFileList>(
                        client, GFileListRequest{.corpora = "user",
                                                 .includeItemsFromAllDrives = false,
                                                 .orderBy = "createdTime desc",
                                                 .pageSize = (isLast) ? 20u : 1u,
                                                 .q = q,
                                                 .supportsAllDrives = true,
                                                 .fields = "nextPageToken, files(name, id, createdTime)"});
                }
                if (nextDir->files.empty()) {
                    spdlog::error("Directory '{}' not found in Google Drive under {}", it->generic_string(),
                                  currentPath.generic_string());
                }
                if (!nextDir->files[0]->id.has_value()) {
                    spdlog::error("Directory ID is missing for '{}'", it->generic_string());
                    return std::nullopt;
                }
                root = nextDir->files[0];
                currentPath /= *it;
            }
        }
        return *nextDir.get();
    }

    GFileList::GFileList(std::weak_ptr<GCloud::Authentication::OAuthAgent> client, const GFileListRequest& request)
        : _client(client) {
        auto params = cpr::Parameters{};
        if (!request.q.empty()) params.Add({"q", request.q});
        params.Add({"pageSize", std::to_string(request.pageSize)});
        if (!request.pageToken.empty()) params.Add({"pageToken", request.pageToken});
        if (!request.orderBy.empty()) params.Add({"orderBy", request.orderBy});
        if (!request.spaces.empty()) params.Add({"spaces", request.spaces});
        params.Add({"includeItemsFromAllDrives", request.includeItemsFromAllDrives ? "true" : "false"});
        params.Add({"supportsAllDrives", request.supportsAllDrives ? "true" : "false"});
        if (!request.includePermissionsForView.empty())
            params.Add({"includePermissionsForView", request.includePermissionsForView});
        if (!request.includeLabels.empty()) params.Add({"includeLabels", request.includeLabels});
        if (!request.corpora.empty()) params.Add({"corpora", request.corpora});
        if (!request.driveId.empty()) params.Add({"driveId", request.driveId});
        if (!request.fields.empty()) params.Add({"fields", request.fields});

        auto response =
            cpr::Get(cpr::Url{"https://www.googleapis.com/drive/v3/files"}, params,
                     cpr::Header{{"Authorization", "Bearer " + _client.lock()->getAccessToken()}}, cpr::VerifySsl(0));

        if (response.status_code != 200) {
            throw std::runtime_error(std::format("Failed to fetch file list: {} - {}\n{}", response.status_code,
                                                 response.reason, response.text));
        }
        auto jsonResponse = nlohmann::json::parse(response.text);
        if (!jsonResponse.contains("files")) {
            throw std::runtime_error("Failed to fetch file list: Invalid JSON Response, missing files");
        }
        if (jsonResponse.contains("nextPageToken")) {
            _nextPageToken = jsonResponse["nextPageToken"].get<std::string>();
        } else {
            _nextPageToken = "";
        }
        for (const nlohmann::json& file : jsonResponse["files"]) {
            std::shared_ptr<GFile> f = std::make_shared<GFile>(_client);
            for (auto& it : file.items()) {
                try {
                    if (it.value().is_string()) {
                        f->setStringField(it.key(), it.value().get<std::string>());
                    } else if (it.value().is_boolean()) {
                        f->setBoolField(it.key(), it.value().get<bool>());
                    }
                } catch (const std::exception& e) {
                    spdlog::error("Error setting field '{}': {}", it.key(), e.what());
                }
            }
            files.push_back(f);
        }
    }

    /*GFile::GFile(std::weak_ptr<GCloud::OAuthAgent> client, const nlohmann::json item) : _client(client) {
        if (item.contains("id")) {
            _fileId = item["id"].get<std::string>();
        }
        if (item.contains("name")) {
            _name = item["name"].get<std::string>();
        }
        if (item.contains("mimeType")) {
            _mimeType = item["mimeType"].get<std::string>();
        }
        if (item.contains("createdTime")) {
            _createdTime = item["createdTime"].get<std::string>();
        }
    }*/

    GFile::GFile(std::weak_ptr<GCloud::Authentication::OAuthAgent> client) : _client(client) {}

    void GDrive::GFile::print(std::ostream& os) {
        for (const auto& [key, value] : _stringFieldsGetterMap) {
            if (value(*this)) {
                os << TAB_SPACE << key << ": " << value(*this).value() << "\n";
            }
        }
        for (const auto& [key, value] : _boolFieldsGetterMap) {
            if (value(*this)) {
                os << TAB_SPACE << key << ": " << (value(*this).value() ? "True" : "False") << "\n";
            }
        }
    }

    void GFileList::print(std::ostream& os) {
        os << "Next Page Token: " << _nextPageToken << "\n";
        os << "Files:\n";
        for (const auto& file : files) {
            file->print(os);
            os << "===============================================================\n";
        }
    }

    void GFile::download(const std::string& path) {
        if (auto client = _client.lock()) {
            if (!id.has_value()) {
                throw std::runtime_error("File download failed: Missing file Id");
            }
            std::filesystem::path finalPath = std::filesystem::path(path);

            if (!finalPath.has_filename()) {
                if (name.has_value()) {
                    finalPath /= name.value();
                } else {
                    finalPath /= id.value();
                }
            }
            std::ofstream outFile(finalPath, std::ios::binary);
            auto response = cpr::Download(
                outFile, cpr::Url{"https://www.googleapis.com/drive/v3/files/" + id.value() + "?alt=media"},
                cpr::Bearer{client->getAccessToken()}, cpr::VerifySsl(0));
            if (response.status_code != 200) {
                throw std::runtime_error(std::format("File download failed: {} - {}\n{}", response.status_code,
                                                     response.reason, response.text));
            }
        } else {
            throw std::runtime_error("File download failed: Client is no longer valid");
        }
    }

    void GFile::setStringField(const std::string_view& field, const std::string_view& value) {
        std::string f;
        f.resize(field.size());
        std::transform(field.begin(), field.end(), f.begin(), ::tolower);
        auto it = _stringFieldsSetterMap.find(f);
        if (it != _stringFieldsSetterMap.end()) {
            it->second(*this, value);
        } else {
            throw std::runtime_error("Invalid string field: " + std::string(field));
        }
    }

    void GFile::setBoolField(const std::string_view& field, bool value) {
        std::string f;
        f.resize(field.size());
        std::transform(field.begin(), field.end(), f.begin(), ::tolower);
        auto it = _boolFieldsSetterMap.find(f);
        if (it != _boolFieldsSetterMap.end()) {
            it->second(*this, value);
        } else {
            throw std::runtime_error("Invalid boolean field: " + std::string(field));
        }
    }

    std::unordered_map<std::string, GFileCapabilities::Type> GFileCapabilities::_capabilitiesMap = {
        {"canchangeviewerscancopycontent", GFileCapabilities::Type::canChangeViewersCanCopyContent},
        {"canmovechildrenoutofdrive", GFileCapabilities::Type::canMoveChildrenOutOfDrive},
        {"canreaddrive", GFileCapabilities::Type::canReadDrive},
        {"canedit", GFileCapabilities::Type::canEdit},
        {"cancopy", GFileCapabilities::Type::canCopy},
        {"cancomment", GFileCapabilities::Type::canComment},
        {"canaddchildren", GFileCapabilities::Type::canAddChildren},
        {"candelete", GFileCapabilities::Type::canDelete},
        {"candownload", GFileCapabilities::Type::canDownload},
        {"canlistchildren", GFileCapabilities::Type::canListChildren},
        {"canremovechildren", GFileCapabilities::Type::canRemoveChildren},
        {"canrename", GFileCapabilities::Type::canRename},
        {"cantrash", GFileCapabilities::Type::canTrash},
        {"canreadrevisions", GFileCapabilities::Type::canReadRevisions},
        {"canreadteamdrive", GFileCapabilities::Type::canReadTeamDrive},
        {"canmoveteamdriveitem", GFileCapabilities::Type::canMoveTeamDriveItem},
        {"canchangecopyrequireswriterpermission", GFileCapabilities::Type::canChangeCopyRequiresWriterPermission},
        {"canmoveitemintoteamdrive", GFileCapabilities::Type::canMoveItemIntoTeamDrive},
        {"canuntrash", GFileCapabilities::Type::canUntrash},
        {"canmodifycontent", GFileCapabilities::Type::canModifyContent},
        {"canmoveitemwithinteamdrive", GFileCapabilities::Type::canMoveItemWithinTeamDrive},
        {"canmoveitemoutofteamdrive", GFileCapabilities::Type::canMoveItemOutOfTeamDrive},
        {"candeletechildren", GFileCapabilities::Type::canDeleteChildren},
        {"canmovechildrenoutofteamdrive", GFileCapabilities::Type::canMoveChildrenOutOfTeamDrive},
        {"canmovechildrenwithinteamdrive", GFileCapabilities::Type::canMoveChildrenWithinTeamDrive},
        {"cantrashchildren", GFileCapabilities::Type::canTrashChildren},
        {"canmoveitemoutofdrive", GFileCapabilities::Type::canMoveItemOutOfDrive},
        {"canaddmydriveparent", GFileCapabilities::Type::canAddMyDriveParent},
        {"canremovemydriveparent", GFileCapabilities::Type::canRemoveMyDriveParent},
        {"canmoveitemwithindrive", GFileCapabilities::Type::canMoveItemWithinDrive},
        {"canshare", GFileCapabilities::Type::canShare},
        {"canmovechildrenwithindrive", GFileCapabilities::Type::canMoveChildrenWithinDrive},
        {"canmodifycontentrestriction", GFileCapabilities::Type::canModifyContentRestriction},
        {"canaddfolderfromanotherdrive", GFileCapabilities::Type::canAddFolderFromAnotherDrive},
        {"canchangesecurityupdateenabled", GFileCapabilities::Type::canChangeSecurityUpdateEnabled},
        {"canacceptownership", GFileCapabilities::Type::canAcceptOwnership},
        {"canreadlabels", GFileCapabilities::Type::canReadLabels},
        {"canmodifylabels", GFileCapabilities::Type::canModifyLabels},
        {"canmodifyeditorcontentrestriction", GFileCapabilities::Type::canModifyEditorContentRestriction},
        {"canmodifyownercontentrestriction", GFileCapabilities::Type::canModifyOwnerContentRestriction},
        {"canremovecontentrestriction", GFileCapabilities::Type::canRemoveContentRestriction},
        {"candisableinheritedpermissions", GFileCapabilities::Type::canDisableInheritedPermissions},
        {"canenableinheritedpermissions", GFileCapabilities::Type::canEnableInheritedPermissions}};

    std::unordered_map<std::string, std::function<void(GFile&, const std::string_view&)>>
        GFile::_stringFieldsSetterMap = {
            {"kind", [](GFile& f, const std::string_view& v) { f.kind = std::string(v); }},
            {"driveid", [](GFile& f, const std::string_view& v) { f.driveId = std::string(v); }},
            {"fileextension", [](GFile& f, const std::string_view& v) { f.fileExtension = std::string(v); }},
            {"md5checksum", [](GFile& f, const std::string_view& v) { f.md5Checksum = std::string(v); }},
            {"mimetype", [](GFile& f, const std::string_view& v) { f.mimeType = std::string(v); }},
            {"thumbnaillink", [](GFile& f, const std::string_view& v) { f.thumbnailLink = std::string(v); }},
            {"iconlink", [](GFile& f, const std::string_view& v) { f.iconLink = std::string(v); }},
            {"headrevisionid", [](GFile& f, const std::string_view& v) { f.headRevisionId = std::string(v); }},
            {"webviewlink", [](GFile& f, const std::string_view& v) { f.webViewLink = std::string(v); }},
            {"webcontentlink", [](GFile& f, const std::string_view& v) { f.webContentLink = std::string(v); }},
            {"size", [](GFile& f, const std::string_view& v) { f.size = std::string(v); }},
            {"foldercolorrgb", [](GFile& f, const std::string_view& v) { f.folderColorRgb = std::string(v); }},
            {"id", [](GFile& f, const std::string_view& v) { f.id = std::string(v); }},
            {"name", [](GFile& f, const std::string_view& v) { f.name = std::string(v); }},
            {"description", [](GFile& f, const std::string_view& v) { f.description = std::string(v); }},
            {"createdtime", [](GFile& f, const std::string_view& v) { f.createdTime = std::string(v); }},
            {"modifiedtime", [](GFile& f, const std::string_view& v) { f.modifiedTime = std::string(v); }},
            {"modifiedbymetime", [](GFile& f, const std::string_view& v) { f.modifiedByMeTime = std::string(v); }},
            {"viewedbymetime", [](GFile& f, const std::string_view& v) { f.viewedByMeTime = std::string(v); }},
            {"sharedwithmetime", [](GFile& f, const std::string_view& v) { f.sharedWithMeTime = std::string(v); }},
            {"quotabytesused", [](GFile& f, const std::string_view& v) { f.quotaBytesUsed = std::string(v); }},
            {"version", [](GFile& f, const std::string_view& v) { f.version = std::string(v); }},
            {"originalfilename", [](GFile& f, const std::string_view& v) { f.originalFilename = std::string(v); }},
            {"fullfileextension", [](GFile& f, const std::string_view& v) { f.fullFileExtension = std::string(v); }},
            {"teamdriveid", [](GFile& f, const std::string_view& v) { f.teamDriveId = std::string(v); }},
            {"thumbnailversion", [](GFile& f, const std::string_view& v) { f.thumbnailVersion = std::string(v); }},
            {"trashedtime", [](GFile& f, const std::string_view& v) { f.trashedTime = std::string(v); }},
            {"resourcekey", [](GFile& f, const std::string_view& v) { f.resourceKey = std::string(v); }},
            {"sha1checksum", [](GFile& f, const std::string_view& v) { f.sha1Checksum = std::string(v); }},
            {"sha256checksum", [](GFile& f, const std::string_view& v) { f.sha256Checksum = std::string(v); }}};

    std::unordered_map<std::string, std::function<void(GFile&, bool)>> GFile::_boolFieldsSetterMap = {
        {"copyrequireswriterpermission", [](GFile& f, bool v) { f.copyRequiresWriterPermission = v; }},
        {"writerscanshare", [](GFile& f, bool v) { f.writersCanShare = v; }},
        {"viewedbyme", [](GFile& f, bool v) { f.viewedByMe = v; }},
        {"shared", [](GFile& f, bool v) { f.shared = v; }},
        {"viewerscancopycontent", [](GFile& f, bool v) { f.viewersCanCopyContent = v; }},
        {"hasthumbnail", [](GFile& f, bool v) { f.hasThumbnail = v; }},
        {"starred", [](GFile& f, bool v) { f.starred = v; }},
        {"trashed", [](GFile& f, bool v) { f.trashed = v; }},
        {"explicitlytrashed", [](GFile& f, bool v) { f.explicitlyTrashed = v; }},
        {"ownedbyme", [](GFile& f, bool v) { f.ownedByMe = v; }},
        {"isappauthorized", [](GFile& f, bool v) { f.isAppAuthorized = v; }},
        {"hasaugmentedpermissions", [](GFile& f, bool v) { f.hasAugmentedPermissions = v; }},
        {"modifiedbyme", [](GFile& f, bool v) { f.modifiedByMe = v; }},
        {"inheritedpermissionsdisabled", [](GFile& f, bool v) { f.inheritedPermissionsDisabled = v; }}};

    std::unordered_map<std::string, std::function<std::optional<std::string>(GFile&)>> GFile::_stringFieldsGetterMap = {
        {"kind", [](GFile& f) { return f.kind; }},
        {"driveid", [](GFile& f) { return f.driveId; }},
        {"fileextension", [](GFile& f) { return f.fileExtension; }},
        {"md5checksum", [](GFile& f) { return f.md5Checksum; }},
        {"mimetype", [](GFile& f) { return f.mimeType; }},
        {"thumbnaillink", [](GFile& f) { return f.thumbnailLink; }},
        {"iconlink", [](GFile& f) { return f.iconLink; }},
        {"headrevisionid", [](GFile& f) { return f.headRevisionId; }},
        {"webviewlink", [](GFile& f) { return f.webViewLink; }},
        {"webcontentlink", [](GFile& f) { return f.webContentLink; }},
        {"size", [](GFile& f) { return f.size; }},
        {"foldercolorrgb", [](GFile& f) { return f.folderColorRgb; }},
        {"id", [](GFile& f) { return f.id; }},
        {"name", [](GFile& f) { return f.name; }},
        {"description", [](GFile& f) { return f.description; }},
        {"createdtime", [](GFile& f) { return f.createdTime; }},
        {"modifiedtime", [](GFile& f) { return f.modifiedTime; }},
        {"modifiedbymetime", [](GFile& f) { return f.modifiedByMeTime; }},
        {"viewedbymetime", [](GFile& f) { return f.viewedByMeTime; }},
        {"sharedwithmetime", [](GFile& f) { return f.sharedWithMeTime; }},
        {"quotabytesused", [](GFile& f) { return f.quotaBytesUsed; }},
        {"version", [](GFile& f) { return f.version; }},
        {"originalfilename", [](GFile& f) { return f.originalFilename; }},
        {"fullfileextension", [](GFile& f) { return f.fullFileExtension; }},
        {"teamdriveid", [](GFile& f) { return f.teamDriveId; }},
        {"thumbnailversion", [](GFile& f) { return f.thumbnailVersion; }},
        {"trashedtime", [](GFile& f) { return f.trashedTime; }},
        {"resourcekey", [](GFile& f) { return f.resourceKey; }},
        {"sha1checksum", [](GFile& f) { return f.sha1Checksum; }},
        {"sha256checksum", [](GFile& f) { return f.sha256Checksum; }}};

    std::unordered_map<std::string, std::function<std::optional<bool>(GFile&)>> GFile::_boolFieldsGetterMap = {
        {"copyrequireswriterpermission", [](GFile& f) { return f.copyRequiresWriterPermission; }},
        {"writerscanshare", [](GFile& f) { return f.writersCanShare; }},
        {"viewedbyme", [](GFile& f) { return f.viewedByMe; }},
        {"shared", [](GFile& f) { return f.shared; }},
        {"viewerscancopycontent", [](GFile& f) { return f.viewersCanCopyContent; }},
        {"hasthumbnail", [](GFile& f) { return f.hasThumbnail; }},
        {"starred", [](GFile& f) { return f.starred; }},
        {"trashed", [](GFile& f) { return f.trashed; }},
        {"explicitlytrashed", [](GFile& f) { return f.explicitlyTrashed; }},
        {"ownedbyme", [](GFile& f) { return f.ownedByMe; }},
        {"isappauthorized", [](GFile& f) { return f.isAppAuthorized; }},
        {"hasaugmentedpermissions", [](GFile& f) { return f.hasAugmentedPermissions; }},
        {"modifiedbyme", [](GFile& f) { return f.modifiedByMe; }},
        {"inheritedpermissionsdisabled", [](GFile& f) { return f.inheritedPermissionsDisabled; }}};

}  // namespace GDrive