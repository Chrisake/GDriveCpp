
#include <iostream>
#include <string>

#include "GDriveCpp/gDrive.h"
#include "GDriveCpp/gFile.h"
#include "logging.hpp"

#include "google_api_key.hpp"

int main() {
    utils::logging::LogConsole logConsole(
        "GDriveTest", 0, [](const std::string& msg) { std::cout << "Log message: " << msg << std::endl; });
    logConsole.create();
    std::string clientId = std::string(secrets::clientId);
    std::string clientSecret = std::string(secrets::clientSecret);
    std::shared_ptr<GCloud::Authentication::OAuthAgent> client =
        std::make_shared<GCloud::Authentication::OAuthAgent>(clientId, clientSecret);
    try {
        std::string token = client->getAccessToken();
        std::cout << "Authentication successful!" << std::endl;
        std::cout << "Token: " << token << std::endl;
        GDrive::GFileList list(client, {.corpora = "user",
                                        .includeItemsFromAllDrives = false,
                                        .orderBy = "createdTime desc",
                                        .pageSize = 10,
                                        .q = "'17NYFkn56HVNGp8twCd-cls5Rb9HM_hwI' in parents",
                                        .supportsAllDrives = true,
                                        .fields = "nextPageToken, files(name, id, createdTime)"});
        list.print(std::cout);
        if (!list.files.empty()) {
            list.files[0]->download();
        }
    } catch (const std::exception& e) {
        std::cout << "Authentication failed: " << e.what() << std::endl;
    }
    return 0;
}
